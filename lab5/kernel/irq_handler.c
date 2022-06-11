#include "kernel/irq_handler.h"
struct softirq_status softirq_s = {
    .pending  = 0,
    .in_softirq = 0
};

irq_funcptr softirq_vec[END_OF_LIST] = {
    timer_softirq_callback,
    mini_uart_rx_softirq_callback,
};
uint64_t irq_count[END_OF_LIST] = {0, 0, 0};

void enter_softirq(){
   softirq_s.in_softirq = 1; 
}

void exit_softirq(){
   softirq_s.in_softirq = 0; 
}

uint8_t in_softirq(){
    return softirq_s.in_softirq; 
}

uint8_t has_softirq_pending(){
    return softirq_s.pending ? 1 : 0;
} 

uint16_t get_softirq_pending(){
    return softirq_s.pending;
}

void set_softirq_pending(uint16_t val){
    softirq_s.pending = val;
}


void add_softirq_task(uint32_t softirq_nr){
    softirq_s.pending |= (1 << softirq_nr);
}

void do_softirq(){
    uint16_t try = 0;
    volatile uint16_t pending;
    uint16_t softirq_bit;
    irq_funcptr softirq_handler;

    while(try < MAX_SOFTIRQ_TRY){
        pending = get_softirq_pending();
        set_softirq_pending(0);

        local_irq_enable();
        while(pending){
            softirq_bit = ffs16(pending);
            softirq_handler = softirq_vec[softirq_bit];
            softirq_handler();
            pending &= ~(1 << softirq_bit);
        }
        local_irq_disable();
        try++;
    }
}

void do_irq(uint32_t nr, irq_funcptr do_hardirq,irq_funcptr enable_device_irq , irq_funcptr disable_device_irq){
    disable_device_irq();
    irq_count[nr]++;
    add_softirq_task(nr);
    do_hardirq();

    if(!in_softirq() && has_softirq_pending()){
        enter_softirq();
        do_softirq();
        exit_softirq();
    }
    
    enable_device_irq();
}

void irq_handler(){
    uint32_t irq_pending_1 = IO_MMIO_read32(IRQ_PENDING_1);
    uint32_t core0_irq_source = IO_MMIO_read32(CORE0_IRQ_SOURCE);
    uint32_t auxirq, uart_irq_type;
    /*
    uint32_t irq_pending_2 = IO_MMIO_read32(IRQ_PENDING_2);
    uint32_t irq_basic_pending = IO_MMIO_read32(IRQ_BASIC_PENDING);
    uint32_t core1_irq_source = IO_MMIO_read32(CORE1_IRQ_SOURCE);
    uint32_t core2_irq_source = IO_MMIO_read32(CORE2_IRQ_SOURCE);
    uint32_t core3_irq_source = IO_MMIO_read32(CORE3_IRQ_SOURCE);
    uint32_t core0_fiq_source = IO_MMIO_read32(CORE0_FIQ_SOURCE);
    uint32_t core1_fiq_source = IO_MMIO_read32(CORE1_FIQ_SOURCE);
    uint32_t core2_fiq_source = IO_MMIO_read32(CORE2_FIQ_SOURCE);
    uint32_t core3_fiq_source = IO_MMIO_read32(CORE3_FIQ_SOURCE);
    */
    

    if(core0_irq_source & 2){
        //core timer interrupt
        do_irq(CORE0_TIMER, core_timer_irq_handler, enable_core_timer_irq, disable_core_timer_irq);
        schedule();
    }else if(irq_pending_1 & (1 << 29)){
        auxirq = IO_MMIO_read32(AUX_IRQ); 
        if(auxirq & 1){
            uart_irq_type = (IO_MMIO_read32(AUX_MU_IIR_REG) >> 1) & (0b11);
            if(uart_irq_type == RX){
                // Receiver holds valid byte
                do_irq(MINI_UART_RX, mini_uart_irq_read, enable_mini_uart_rx_irq, disable_mini_uart_rx_irq);
            }else{
                irq_count[UNKNOWN_IRQ]++;
                LOG("UART interrupt");
                while(1);
            }
        }else{
            irq_count[UNKNOWN_IRQ]++;
            LOG("Unkown AUX interrupt, DAIF: %x", get_DAIF());
        }
    }
    else{
        irq_count[UNKNOWN_IRQ]++;
        LOG("Unkown interrupt");
    }
}

void err_handler(uint64_t type, uint64_t esr, uint64_t elr, uint64_t spsr_el1, uint64_t sp_el0, uint64_t sp){
    struct task_struct *cur = get_current();
    write_str("unkown irq count: ");
    write_hex(irq_count[UNKNOWN_IRQ]);
    write_str("\r\n");

    write_str("pid: ");
    write_hex(cur->thread_info.pid);
    write_str("\r\n");
    
    write_str("trap frame: ");
    write_hex((uint64_t)get_trap_frame(cur));
    write_str("\r\n");

    write_str("cur->ctx.lr: ");
    write_hex(cur->ctx.lr);
    write_str("\r\n");

    write_str("cur->ctx.sp: ");
    write_hex(cur->ctx.sp);
    write_str("\r\n");

    write_str("cur->ctx.fp: ");
    write_hex(cur->ctx.fp);
    write_str("\r\n");

    write_str("kernel stack: ");
    write_hex((uint64_t)cur->stack);
    write_str("\r\n");

    write_str("type: ");
    write_hex(type);
    write_str("\r\n");

    write_str("esr_el1: ");
    write_hex(esr);
    write_str("\r\n");

    write_str("elr_el1: ");
    write_hex(get_ELR_EL1());
    write_str("\r\n");

    write_str("spsr_el1: ");
    write_hex(spsr_el1);
    write_str("\r\n");

    write_str("sp_el0: ");
    write_hex(sp_el0);
    write_str("\r\n");

    write_str("sp: ");
    write_hex(sp);
    write_str("\r\n");

    while(1);
}

#include "exception.h"
#include "system_call.h"

void syn_handler () {

    unsigned long esr_el1;
    unsigned long elr_el1;
    unsigned long spsr_el1;

    // Get EL1 registers
    asm volatile("mrs %0,  esr_el1" : "=r"(esr_el1) : );
    asm volatile("mrs %0,  elr_el1" : "=r"(elr_el1) : );
    asm volatile("mrs %0, spsr_el1" : "=r"(spsr_el1) : );

    uart_puts("ESR_EL1 : 0x");
    uart_hex(esr_el1);
    uart_puts("\n");
    uart_puts("ELR_EL1 : 0x");
    uart_hex(elr_el1);
    uart_puts("\n");
    uart_puts("SPSR_EL1: 0x");
    uart_hex(spsr_el1);
    uart_puts("\n");
    uart_puts("----------------------------\n");

    return;
}

void irq_handler () {

    unsigned int is_timer_irq;
    unsigned int is_uart_irq;

    is_timer_irq = (*CORE_0_IRQ_SOURCE & (1 << 1));
    is_uart_irq  = (*IRQ_1_PENDING & (1 << 29));

    if (is_timer_irq)
    {
        timer_irq_handler();
    }
    else if (is_uart_irq)
    {
        uart_irq_handler();
    }

    return;
}

void timer_irq_handler () {

    unsigned long c_time;

    // Set next expire time
    set_next_timeout(2);

    // Get current time
    c_time = get_time();

    // Print prompt
    uart_puts("current time : ");
    uart_hex(c_time);
    uart_puts("secs.\n");

    return;
}

void undefined_handler () {
    return;
}

void enable_interrupt () {

    asm volatile ("msr DAIFClr, 0xf");   

    return;
}

void disable_interrupt () {

    asm volatile ("msr DAIFSet, 0xf");
   
    return;
}

void set_next_timeout (unsigned int seconds) {

    // Set next expire time
    asm volatile ("mrs x2, cntfrq_el0");
    asm volatile ("mul x1, x2, %0" :: "r"(seconds));
    asm volatile ("msr cntp_tval_el0, x1");

    return;
}
/* lab5 b2 */
// void lower_syn_handler(unsigned long spsr_el1, unsigned long elr_el1, unsigned long esr_el1, trapframe_t *tf)
void lower_syn_handler(unsigned long esr_el1, unsigned long elr_el1, trapframe_t *tf)
{
    unsigned long esr = esr_el1;
    if (((esr >> 26) & 0x3f) == 0x15) 
    {
        unsigned long svc = esr & 0x1ffffff;
        unsigned long sys_call_num = tf->x[8];
        uart_puts("sys call num : ");
        uart_put_int(sys_call_num);
        uart_puts("\r\n");
        if(svc == 0)
        {
            switch(sys_call_num)
            {
                case 0:
                    uart_puts("start sys_getpid\r\n");
                    sys_get_pid(tf);
                    // x[0] = sys_getpid();
                    // schedule();
                    uart_puts("end sys_getpid\r\n");
                    break;
                case 1:
                    // uart_puts("start sys_uartread\r\n");
                    sys_uart_read(tf); 
                    // uart_puts("end sys_uartread\r\n");
                    break;
                case 2:
                    // uart_puts("start sys_uartwrite\r\n");
                    sys_uart_write(tf);
                    // enable_interrupt();
                    // disable_interrupt();
                    // uart_puts("end sys_uartwrite\r\n");
                    break;
                case 3:
                    sys_exec(tf);
                    // thread_schedule();
                    break;
                case 4:
                    sys_fork(tf);
                    // thread_schedule();
                    break;
                // case 5:
                //     sys_exit(tf);
                //     // thread_schedule();
                //     break;
                // case 6:
                //     sys_mbox_call(tf);
                //     // thread_schedule();
                //     break;
                // case 7:
                //     sys_kill(tf);
                //     // thread_schedule();
                //     break;
                default:
                    uart_puts("unknown svc!\n");
                    break;
            }
        }
    }
    enable_interrupt();
}
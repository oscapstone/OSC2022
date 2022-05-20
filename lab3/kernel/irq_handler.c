#include "peripherals/iomapping.h"
#include "kernel/timer.h"
#include "peripherals/mini_uart.h"
#include "types.h"
#include "lib/print.h"
#include "debug/debug.h"
#include "asm.h"

typedef void (*irq_funcptr)(void);

void do_irq(irq_funcptr do_hardirq,irq_funcptr enable_device_irq , irq_funcptr disable_device_irq){
    disable_device_irq();
    do_hardirq();
    enable_device_irq();
}

void irq_handler(){
    uint32_t irq_pending_1 = IO_MMIO_read32(IRQ_PENDING_1);
    uint32_t core0_irq_source = IO_MMIO_read32(CORE0_INTERRUPT_SOURCE);
    uint32_t auxirq, uart_irq_type;
    if(core0_irq_source & 2){
        //core timer interrupt
        do_irq(core_timer_irq_handler, enable_core_timer_irq, disable_core_timer_irq);
    }else if(irq_pending_1 & (1 << 29)){
        auxirq = IO_MMIO_read32(AUX_IRQ); 
        if(auxirq & 1){
            uart_irq_type = (IO_MMIO_read32(AUX_MU_IIR_REG) >> 1) & (0b11);
            if(uart_irq_type == RX){
                // Receiver holds valid byte
                do_irq(mini_uart_irq_read, enable_mini_uart_rx_irq, disable_mini_uart_rx_irq);
            }else if(uart_irq_type == TX){
                // Transmit holding register empty
                do_irq(mini_uart_irq_write, enable_mini_uart_tx_irq, disable_mini_uart_tx_irq);
            }else{
                LOG("UART interrupt");
            }
        }else{
            LOG("Unkown AUX interrupt, DAIF: %x", get_DAIF());
        }
    }
    else{
        LOG("Unkown interrupt, DAIF: %x", get_DAIF());
    }
}


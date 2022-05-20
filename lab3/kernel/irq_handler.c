#include "peripherals/iomapping.h"
#include "kernel/timer.h"
#include "peripherals/mini_uart.h"
#include "utils.h"
#include "types.h"
#include "debug/debug.h"
#include "asm.h"
void irq_handler(){
    uint32_t irq_pending_1 = IO_MMIO_read32(IRQ_PENDING_1);
    uint32_t core0_irq_source = IO_MMIO_read32(CORE0_INTERRUPT_SOURCE);
    uint32_t auxirq, uart_irq_type;
    if(core0_irq_source & 2){
        core_timer_irq_handler();
    }else if(irq_pending_1 & (1 << 29)){
        auxirq = IO_MMIO_read32(AUX_IRQ); 
        if(auxirq & 1){
            uart_irq_type = (IO_MMIO_read32(AUX_MU_IIR_REG) >> 1) & (0b11);
            if(uart_irq_type == RX){
                disable_mini_uart_irq(RX);
                // Receiver holds valid byte
                mini_uart_irq_read();
                enable_mini_uart_irq(RX);
            }else if(uart_irq_type == TX){
                // Transmit holding register empty
                disable_mini_uart_irq(TX);
                mini_uart_irq_write();
                if(mini_uart_get_tx_len() > 0){
                    enable_mini_uart_irq(TX);
                }
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

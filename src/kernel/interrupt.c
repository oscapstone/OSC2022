#include <interrupt.h>
#include <uart.h>
#include <stdint.h>
#include <string.h>
#include <timer.h>
#include <mmio.h>

uint32_t interrupt_depth;

void interrupt_irq_handler()
{

    uint32_t irq_pend_base_reg = mmio_load(ARMINT_IRQ_PEND_BASE_REG);
    uint32_t irq_pend1_reg = mmio_load(ARMINT_IRQ_PEND1_REG);
    uint32_t irq_pend2_reg = mmio_load(ARMINT_IRQ_PEND2_REG);
    uint32_t core0_int_src = mmio_load(CORE0_INTERRUPT_SOURCE);
    uint32_t aux_mu_iir_reg = mmio_load(AUX_MU_IIR_REG);
    
    /*
    uart_puts("IRQ Handler.");

    uart_print("GPU IRQ pend base: 0x");
    uart_putshex((uint64_t)irq_pend_base_reg);
    
    uart_print("GPU IRQ pend 1: 0x");
    uart_putshex((uint64_t)irq_pend1_reg);
    
    uart_print("GPU IRQ pend 2: 0x");
    uart_putshex((uint64_t)irq_pend2_reg);
    
    uart_print("Core0 interrupt source: 0x");
    uart_putshex((uint64_t)core0_int_src);
    
    uart_print("AUX_MU_IIR_REG: 0x");
    uart_putshex((uint64_t)aux_mu_iir_reg);
    */

    if(core0_int_src & 0b10){ //CNTPNSIRQ interrupt
        coretimer_el0_handler();
    }
    if(aux_mu_iir_reg&0b100){
        //uart_puts("uart read interrupt!!");
        uart_interrupt_handler();
    }
    //uart_puts("irq interrupt");
}

void not_implemented_interrupt()
{
    uart_puts("not_implemented_interrupt");
}

void interrupt_fiq_handler()
{
    uart_puts("FIQ Handler.");
    while(1);
}

void interrupt_enable()
{
    if(interrupt_depth==0){
        //uart_puts("enable interrupt");
        //uart_print("interrupt_depth: 0x");
        //uart_putshex(interrupt_depth);
        asm("msr DAIFClr, 0xf");
    }
    interrupt_depth++;
}

void interrupt_disable()
{
    interrupt_depth--;
    if(interrupt_depth==0)asm("msr DAIFSet, 0xf");
}
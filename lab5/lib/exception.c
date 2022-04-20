#include "exception.h"
#include "mini_uart.h"
#include "mailbox.h"
#include "utils.h"
#include "timer.h"
#include "peripherals/exception.h"
#include "peripherals/mini_uart.h"

void enable_interrupt() {asm volatile("msr DAIFClr, 0xf");}
void disable_interrupt() {asm volatile("msr DAIFSet, 0xf");}

void exception_entry() {
    
    disable_interrupt();
    unsigned long long spsr_el1;
	asm volatile("mrs %0, SPSR_EL1\n\t" : "=r" (spsr_el1) :  : "memory");

    unsigned long long elr_el1;
	asm volatile("mrs %0, ELR_EL1\n\t" : "=r" (elr_el1) :  : "memory");

    unsigned long long esr_el1;
	asm volatile("mrs %0, ESR_EL1\n\t" : "=r" (esr_el1) :  : "memory");
    
    printf("spsr_el1: %x\n", spsr_el1);
    printf("elr_el1: %x\n", elr_el1);
    printf("esr_el1: %x\n", esr_el1);

    while(1);

}

void irq_exc_router() {

    disable_interrupt();
    if (get32(IRQ_PENDING_1)&IRQ_PENDING_1_AUX_INT && get32(CORE0_INTERRUPT_SRC)&INTERRUPT_SOURCE_GPU) {
        //async_uart_handler();
        printf("async uart interrupt handler not implemented.\n");
    } else if (get32(CORE0_INTERRUPT_SRC)&INTERRUPT_SOURCE_CNTPNSIRQ) {
        pop_timer();
    }
    enable_interrupt();

}

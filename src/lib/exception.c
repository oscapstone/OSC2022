#include "exception.h"

void no_exception_handle() {
    return;
}

void lowerEL_sync_interrupt_handle() {

    // get reg info
    unsigned long spsr_el1, elr_el1, esr_el1;
    asm volatile("mrs %0, spsr_el1" : "=r"(spsr_el1));
	asm volatile("mrs %0, elr_el1" : "=r"(elr_el1));
	asm volatile("mrs %0, esr_el1" : "=r"(esr_el1));

    // print regs' info
    uart_puts("SPSR_EL1: 0x");
    uart_hex(spsr_el1);
    uart_puts("\r\n");

    uart_puts("ELR_EL1: 0x");
    uart_hex(elr_el1);
    uart_puts("\r\n");

    uart_puts("ESR_EL1: 0x");
    uart_hex(esr_el1);
    uart_puts("\r\n");

    return;
}

void irq_router() {
    unsigned int core_irq = (*CORE0_INTR_SRC & (1 << 1));
    unsigned int uart_irq = (*IRQ_PENDING_1 & AUX_IRQ);

    if (core_irq) {
        core_timer_interrupt();
    }
    else if (uart_irq) {
        mini_uart_interrupt_handler();
    }
}
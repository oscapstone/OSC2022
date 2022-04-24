#include "exception.h"

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
//////////////////////////////////////////////////
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
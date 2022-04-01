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
    
    uart_send_string("spsr_el1: ");
    printhex(spsr_el1);
    uart_send_string("\n");
    
    uart_send_string("elr_el1: ");
    printhex(elr_el1);
    uart_send_string("\n");
    
    uart_send_string("esr_el1: ");
    printhex(esr_el1);
    uart_send_string("\n");
    while(1);
}

void sync_64_router() {
    disable_interrupt();
    unsigned long long spsr_el1;
	asm volatile("mrs %0, SPSR_EL1\n\t" : "=r" (spsr_el1) :  : "memory");

    unsigned long long elr_el1;
	asm volatile("mrs %0, ELR_EL1\n\t" : "=r" (elr_el1) :  : "memory");

    unsigned long long esr_el1;
	asm volatile("mrs %0, ESR_EL1\n\t" : "=r" (esr_el1) :  : "memory");
    
    uart_send_string("spsr_el1: ");
    printhex(spsr_el1);
    uart_send_string("\n");
    
    uart_send_string("elr_el1: ");
    printhex(elr_el1);
    uart_send_string("\n");
    
    uart_send_string("esr_el1: ");
    printhex(esr_el1);
    uart_send_string("\n");
    enable_interrupt();
}

void irq_exc_router() {
    disable_interrupt();
    if (get32(IRQ_PENDING_1)&IRQ_PENDING_1_AUX_INT && get32(CORE0_INTERRUPT_SRC)&INTERRUPT_SOURCE_GPU) {

        async_uart_handler();

    } else if (get32(CORE0_INTERRUPT_SRC)&INTERRUPT_SOURCE_CNTPNSIRQ) {

        pop_timer();

    }
    enable_interrupt();
}

void two_btime_handler() {

    time_elapsed();

    asm volatile(
        "mrs x0, cntfrq_el0\n\t"
        "mov x1, 2\n\t"
        "mul x0, x0, x1\n\t"
        "msr cntp_tval_el0, x0\n\t"
    );

}
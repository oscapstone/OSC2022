#include "exception.h"
#include "mini_uart.h"
#include "mailbox.h"
#include "utils.h"
#include "peripherals/exception.h"
#include "peripherals/mini_uart.h"

void post_btime() {

    unsigned int sec;

    unsigned int cntpct_el0;
    asm volatile("mrs %0, CNTPCT_EL0\n\t" : "=r" (cntpct_el0) :  : "memory");

    unsigned int cntfrq_el0;
    asm volatile("mrs %0, CNTFRQ_EL0\n\t" : "=r" (cntfrq_el0) :  : "memory");

    sec = cntpct_el0 / cntfrq_el0;

    uart_send_string("hex-format seconds after boot:");
    printhex(sec);
    uart_send_string("\n");

}

void exception_entry() {
    
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
    
}

void irq_exc_router() {
    
    if (get32(IRQ_PENDING_1)&IRQ_PENDING_1_AUX_INT && get32(CORE0_INTERRUPT_SRC)&INTERRUPT_SOURCE_GPU) {

        async_uart_handler();

    } else if (get32(CORE0_INTERRUPT_SRC)&INTERRUPT_SOURCE_CNTPNSIRQ) {

        post_btime();
        asm volatile(
            "mrs x0, cntfrq_el0\n\t"
            "mov x1, 2\n\t"
            "mul x0, x0, x1\n\t"
            "msr cntp_tval_el0, x0\n\t"
        );

    }

}

#include "utils.h"
#include "mini_uart.h"
#include "peripherals/mini_uart.h"
#include "exception.h"
#include "sysreg.h"
#include "timer.h"


void enable_interrupt() { asm volatile("msr DAIFClr, 0xf"); }
void disable_interrupt() { asm volatile("msr DAIFSet, 0xf"); }

void lower_sync_handler() {
	disable_interrupt();
	dumpState();
	enable_interrupt();
}

void lower_iqr_handler() {
	disable_interrupt();
	asm volatile("mrs x0, cntfrq_el0	\n");
	asm volatile("add x0, x0, x0		\n");
	asm volatile("msr cntp_tval_el0, x0	\n");
	unsigned long cntpct,cntfrq,tmp;
	asm volatile("mrs %0, cntpct_el0	\n":"=r"(cntpct):);
	asm volatile("mrs %0, cntfrq_el0	\n":"=r"(cntfrq):);
	tmp=cntpct * 10 / cntfrq;
	uart_printf("--------------------\n");
	uart_printf("Time Elapsed: %d.%ds\n", tmp/10, tmp%10);
	uart_printf("--------------------\n");
	enable_interrupt();
}

void curr_sync_handler() {
	disable_interrupt();
	error_handler();
	enable_interrupt();
}

void curr_iqr_handler() {
	disable_interrupt();
    unsigned int uart = (*IRQ_PENDING_1 & AUX_IRQ);
    if (uart)
		uart_handler();
    enable_interrupt();
}

void error_handler() {
	disable_interrupt();
	uart_printf("[ERROR] unknown exception...\n");
	dumpState();
	while(1){}
}

void dumpState() {
	unsigned long esr,elr,spsr;
	asm volatile("mrs %0, esr_el1	\n":"=r"(esr):);
	asm volatile("mrs %0, elr_el1	\n":"=r"(elr):);
	asm volatile("mrs %0, spsr_el1	\n":"=r"(spsr):);
	uart_printf("--------------------\n");
	uart_printf("SPSR: 0x%x\n",spsr);
	uart_printf("ELR: 0x%x\n",elr);
	uart_printf("ESR: 0x%x\n",esr);
	uart_printf("--------------------\n");
}
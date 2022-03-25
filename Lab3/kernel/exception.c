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
	unsigned long cntpct = read_sysreg(cntpct_el0);
	unsigned long cntfrq = read_sysreg(cntfrq_el0);
	unsigned long tmp = cntpct * 10 / cntfrq;
	uart_printf("--------------------\n");
	uart_printf("Time Elapsed: %d.%ds\n", tmp/10, tmp%10);
	uart_printf("--------------------\n");
	write_sysreg(cntp_tval_el0, cntfrq << 1);
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
	unsigned long esr = read_sysreg(esr_el1);
	unsigned long elr = read_sysreg(elr_el1);
	unsigned long spsr = read_sysreg(spsr_el1);
	uart_printf("--------------------\n");
	uart_printf("SPSR: 0x%x\n", spsr);
	uart_printf("ELR: 0x%x\n", elr);
	uart_printf("ESR: 0x%x\n", esr);
	uart_printf("--------------------\n");
}
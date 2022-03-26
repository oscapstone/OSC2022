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
	pop_timer();
	enable_interrupt();
}

void curr_sync_handler() {
	disable_interrupt();
	error_handler();
	enable_interrupt();
}

void curr_iqr_handler() {
	disable_interrupt();
    if (*IRQ_PENDING_1 & AUX_IRQ)
		uart_handler();
	else
		pop_timer();
    enable_interrupt();
}

void error_handler() {
	disable_interrupt();
	uart_send_string("[ERROR] unknown exception...\n");
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
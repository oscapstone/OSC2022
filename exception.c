#include "exception.h"

#include "gpio.h"
#include "uart.h"
#include "utils.h"

#define AUX_IRQ (1 << 29)
#define IRQ_PENDING_1			((volatile unsigned int*)(MMIO_BASE+0x0000b204))
#define CORE0_INTERRUPT_SOURCE	((volatile unsigned int *)(0x40000060))


void enable_current_interrupt() { asm volatile("msr DAIFClr, 0xf"); }

void disable_current_interrupt() { asm volatile("msr DAIFSet, 0xf"); }

void enable_timer_interrupt() {
	asm volatile("mov x0, 1				\n");
	asm volatile("msr cntp_ctl_el0, x0	\n"); // enable
	asm volatile("mov x0, 2				\n");
	asm volatile("ldr x1, =0x40000040	\n"); // CORE0_TIMER_IRQ_CTRL
	asm volatile("str x0, [x1]			\n"); // unmask timer interrupt
	}

void disable_timer_interrupt() {
	asm volatile("mov x0, 0				\n");
	asm volatile("msr cntp_ctl_el0, x0	\n"); // enable
	asm volatile("mov x0, 0				\n");
	asm volatile("ldr x1, =0x40000040	\n"); // CORE0_TIMER_IRQ_CTRL
	asm volatile("str x0, [x1]			\n"); // mask timer interrupt
	}

void dump() {
	unsigned long spsr_el1, elr_el1, esr_el1;
	asm volatile("mrs %0, spsr_el1	\n": "=r"(spsr_el1):);
	asm volatile("mrs %0, elr_el1	\n": "=r"(elr_el1):);
	asm volatile("mrs %0, esr_el1	\n": "=r"(esr_el1):);

	printf("spsr_el1:\t0x%x\n", spsr_el1);
	printf("elr_el1:\t0x%x\n", elr_el1);
	printf("esr_el1:\t0x%x\n", esr_el1);
	printf("\n");
}

void set_time(unsigned int duration) {
	unsigned long cntfrq_el0;
	asm volatile("mrs %0, cntfrq_el0	\n": "=r"(cntfrq_el0):);
	asm volatile("msr cntp_tval_el0, %0	\n":: "r"(cntfrq_el0 * duration));
}

unsigned long get_time10() {
	unsigned long cntpct_el0, cntfrq_el0;
	asm volatile("mrs %0, cntpct_el0	\n": "=r"(cntpct_el0):);
	asm volatile("mrs %0, cntfrq_el0	\n": "=r"(cntfrq_el0):);
	return cntpct_el0*10 / cntfrq_el0;
}

void handle_timer0_irq() {
	unsigned long timer;
	set_time(2);	
	timer = get_time10();
	printf("time: %d.%ds\n", timer/10, timer%10);
	printf("\n");
}

void handle_timer1_irq() {
	// unsigned long timer;
	// timer = get_time10();
	// char *tmp;
	// tmp = scanf("current time: %d.%ds\nMESSAGE\n", timer/10, timer%10);
	
	enable_transmit_interrupt();
	disable_timer_interrupt();
}

void lower_sync_entry() {
	dump();
}

void lower_irq_entry() {
	// disable_current_interrupt();
	if (*CORE0_INTERRUPT_SOURCE & 0x2) {
		// Lower Core Timer Interrupt
		handle_timer0_irq();
	}
	else if (*IRQ_PENDING_1 & AUX_IRQ) {
		// Lower mini UART’s Interrupt
		handle_uart_irq();
	}
	// enable_current_interrupt();
}

void invalid_entry() {
	dump();
		printf("No such exception\n");
	while (1)
        ;
}

void current_irq_entry() {
	// disable_current_interrupt();
	if (*CORE0_INTERRUPT_SOURCE & 0x2) {
		// current Core Timer Interrupt
		handle_timer1_irq();
	}
	else if (*IRQ_PENDING_1 & AUX_IRQ) {
		// current mini UART’s Interrupt
		handle_uart_irq();
	}
	// enable_current_interrupt();
}

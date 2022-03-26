#include "uart.h"

void dump() {
	unsigned long spsr_el1, elr_el1, esr_el1;
	asm volatile("mrs %0, spsr_el1	\n":"=r"(spsr_el1):);
	asm volatile("mrs %0, elr_el1	\n":"=r"(elr_el1):);
	asm volatile("mrs %0, esr_el1	\n":"=r"(esr_el1):);

	printf("spsr_el1:\t0x%x\n", spsr_el1);
	printf("elr_el1:\t0x%x\n", elr_el1);
	printf("esr_el1:\t0x%x\n", esr_el1);
	printf("\n");
}

void lower_sync_entry() {
	dump();
}

void lower_irq_entry() {
	asm volatile("mrs x0, cntfrq_el0	\n");
	asm volatile("add x0, x0, x0		\n");
	asm volatile("msr cntp_tval_el0, x0	\n");

	unsigned long cntpct_el0, cntfrq_el0, timer;
	asm volatile("mrs %0, cntpct_el0	\n":"=r"(cntpct_el0):);
	asm volatile("mrs %0, cntfrq_el0	\n":"=r"(cntfrq_el0):);

	timer = cntpct_el0*10 / cntfrq_el0;
	printf("time: %d.%ds\n", timer/10, timer%10);
	printf("\n");
}

void invalid_entry() {
	dump();
		printf("No such exception\n");
	while (1)
        ;
}
#include "exception.h"

#include "gpio.h"
#include "uart.h"
#include "utils.h"
#include "command.h"
#include "timer.h"
#include "mmu.h"
#include "thread.h"

#define AUX_IRQ (1 << 29)
#define IRQ_PENDING_1			((volatile unsigned int*)(MMIO_BASE+0x0000b204))
#define CORE0_TIMER_IRQ_CTRL 	((volatile unsigned int*)(KVA+0x40000040))
#define CORE0_INTERRUPT_SOURCE	((volatile unsigned int *)(KVA+0x40000060))
#define ESR_ELx_EC_SHIFT		26
#define ESR_ELx_EC_SVC64		0x15
#define ESR_ELx_EC_DABT_LOW		0x24

void enable_current_interrupt() {
	asm volatile("msr DAIFClr, 0xf");
	set_time_shift(5);
	enable_timer_interrupt();
}

void disable_current_interrupt() {
	asm volatile("msr DAIFSet, 0xf");
	// set_time_shift(1000000);
	disable_timer_interrupt();
}

void enable_timer_interrupt() {
	asm volatile("mov x0, 1				\n");
	asm volatile("msr cntp_ctl_el0, x0	\n"); // enable
	asm volatile("mov x0, 2				\n");
	// asm volatile("mov x1, %0	\n"::"r"(CORE0_TIMER_IRQ_CTRL)); // CORE0_TIMER_IRQ_CTRL
	asm volatile("ldr x1, =0xFFFF000040000040	\n"); // CORE0_TIMER_IRQ_CTRL
	asm volatile("str x0, [x1]			\n"); // unmask timer interrupt
}

void disable_timer_interrupt() {
	asm volatile("mov x0, 0				\n");
	asm volatile("msr cntp_ctl_el0, x0	\n"); // enable
	asm volatile("mov x0, 0				\n");
	// asm volatile("mov x1, %0	\n"::"r"(CORE0_TIMER_IRQ_CTRL)); // CORE0_TIMER_IRQ_CTRL
	asm volatile("ldr x1, =0xFFFF000040000040	\n"); // CORE0_TIMER_IRQ_CTRL
	asm volatile("str x0, [x1]			\n"); // mask timer interrupt
}

void timer_register() {
	unsigned long tmp;
	asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
	tmp |= 1;
	asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
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

void invalid_entry() {
	dump();
	printf("No such exception\n");
	exec_reboot();
	while (1)
        ;
}

void set_time(unsigned int duration) {
	unsigned long cntfrq_el0;
	asm volatile("mrs %0, cntfrq_el0	\n": "=r"(cntfrq_el0):);
	asm volatile("msr cntp_tval_el0, %0	\n":: "r"(cntfrq_el0 * duration));
}
void set_time_shift(unsigned int duration) {
	unsigned long cntfrq_el0;
	asm volatile("mrs %0, cntfrq_el0	\n": "=r"(cntfrq_el0):);
	asm volatile("msr cntp_tval_el0, %0	\n":: "r"(cntfrq_el0 >> duration));
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
	handle_timer_list();
}

void lower_svc(Trap_Frame *tpf) {
    unsigned long long syscall_svc = tpf->x8;

	#ifdef ASYNC_UART
	enable_current_interrupt();
	#endif

    if (syscall_svc == 0) {
        getpid(tpf);
    }
    else if(syscall_svc == 1) {
        uartread(tpf,(char *) tpf->x0, tpf->x1);
    }
    else if (syscall_svc == 2) {
        uartwrite(tpf,(char *) tpf->x0, tpf->x1);
    }
    else if (syscall_svc == 3) {
        exec(tpf,(char *) tpf->x0, (char **)tpf->x1);
    }
    else if (syscall_svc == 4) {
        fork(tpf);
    }
    else if (syscall_svc == 5) {
        exit(tpf,tpf->x0);
    }
    else if (syscall_svc == 6) {
        syscall_mbox_call(tpf,(unsigned char)tpf->x0, (unsigned int *)tpf->x1);
    }
    else if (syscall_svc == 7) {
        kill(tpf, (int)tpf->x0);
    }
    else if (syscall_svc == 11) {
        open(tpf, (const char *)tpf->x0, tpf->x1);
    }
    else if (syscall_svc == 12) {
        close(tpf, tpf->x0);
    }
    else if (syscall_svc == 13) {
        write(tpf, tpf->x0, (const void *)tpf->x1, tpf->x2);
    }
    else if (syscall_svc == 14) {
        read(tpf, tpf->x0, (void *)tpf->x1, tpf->x2);
    }
    else if (syscall_svc == 15) {
        mkdir(tpf, (const char *)tpf->x0, tpf->x1);
    }
    else if (syscall_svc == 16) {
        mount(tpf, (const char *)tpf->x0, (const char *)tpf->x1, (const char *)tpf->x2, tpf->x3, (const void *)tpf->x4);
    }
    else if (syscall_svc == 17) {
        chdir(tpf, (const char *)tpf->x0);
    }
	else {
		printf("unknown system call:%d\n",syscall_svc);
	}
}

void lower_sync_entry(Trap_Frame *tpf) {
	unsigned long esr;
	asm volatile("mrs %0, esr_el1	\n":"=r"(esr):);
	esr = (esr >> ESR_ELx_EC_SHIFT) & 0x3f;

	if(esr == ESR_ELx_EC_SVC64)
		lower_svc(tpf);
	else if(esr == ESR_ELx_EC_DABT_LOW)
		lower_data_abort_handler();
	else {
		printf("lower sync unknown reason\nplease check esr_el1\n");
		invalid_entry();
	}
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

void schedule_irq() {
	// printf("irp");
	if (*CORE0_INTERRUPT_SOURCE & 0x2) {
		// current Core Timer Interrupt
		// uart_puts("second");
		set_time_shift(5);
		enable_current_interrupt();
		schedule();
		
	}
	else if (*IRQ_PENDING_1 & AUX_IRQ) {
		// current mini UART’s Interrupt
		handle_uart_irq();
	}
}
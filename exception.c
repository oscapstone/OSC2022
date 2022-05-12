#include "exception.h"

#include "gpio.h"
#include "uart.h"
#include "utils.h"
#include "timer.h"
#include "thread.h"

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

void lower_sync_entry(Trap_Frame *tpf)
{

    unsigned long long syscall_no = tpf->x8;

	#ifdef ASYNC_UART
	enable_current_interrupt();
	#endif

	// printf("--%x--\n", syscall_no);
	// unsigned long esr;
	// asm volatile("mrs %0, esr_el1	\n":"=r"(esr):);
	// printf("--%x--\n", esr);
	// if(((esr>>26)&0x3f)!=0x15){

	// }
    if (syscall_no == 0)
    {
        getpid(tpf);
    }
    else if(syscall_no == 1)
    {
        uartread(tpf,(char *) tpf->x0, tpf->x1);
    }
    else if (syscall_no == 2)
    {
        uartwrite(tpf,(char *) tpf->x0, tpf->x1);
    }
    else if (syscall_no == 3)
    {
        exec(tpf,(char *) tpf->x0, (char **)tpf->x1);
    }
    else if (syscall_no == 4)
    {
        fork(tpf);
    }
    else if (syscall_no == 5)
    {
        exit(tpf,tpf->x0);
    }
    else if (syscall_no == 6)
    {
        syscall_mbox_call(tpf,(unsigned char)tpf->x0, (unsigned int *)tpf->x1);
    }
    else if (syscall_no == 7)
    {
        kill(tpf, (int)tpf->x0);
    }
    // else if (syscall_no == 8)
    // {
    //     signal_register(tpf->x0, (void (*)())tpf->x1);
    // }
    // else if (syscall_no == 9)
    // {
    //     signal_kill(tpf->x0, tpf->x1);
    // }
    // else if (syscall_no == 50)
    // {
    //     sigreturn();
    // }

    /*
    unsigned long long spsr_el1;
	__asm__ __volatile__("mrs %0, SPSR_EL1\n\t" : "=r" (spsr_el1));

    unsigned long long elr_el1;
	__asm__ __volatile__("mrs %0, ELR_EL1\n\t" : "=r" (elr_el1));

    unsigned long long esr_el1;
	__asm__ __volatile__("mrs %0, ESR_EL1\n\t" : "=r" (esr_el1));*/

    //uart_printf("exception sync_el0_64_router -> spsr_el1 : 0x%x, elr_el1 : 0x%x, esr_el1 : 0x%x\r\n",spsr_el1,elr_el1,esr_el1);
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
#include "timer.h"

void core_timer_enable() {
    asm volatile ("mov x1, 1");
    asm volatile ("msr cntp_ctl_el0, x1"); // enable (check the last bit)
    asm volatile ("mrs x1, cntfrq_el0");
    asm volatile ("msr cntp_tval_el0, x1"); // set expired time
    asm volatile ("mov x1, 2");
    asm volatile ("ldr x4, =0x40000040");
    asm volatile ("str w1, [x4]"); // unmask timer interrupt
}

void core_timer_interrupt() {
    unsigned long long cntpct_el0, cntfrq_el0, time;
    asm volatile ("mrs %0, cntpct_el0" : "=r" (cntpct_el0));
    asm volatile ("mrs %0, cntfrq_el0" : "=r" (cntfrq_el0));
    time = cntpct_el0 / cntfrq_el0;
    uart_puts("time after booting: ");
    uart_hex(time);
    uart_puts("\n");

    // set next timeout
    volatile unsigned long long next_timeout = 2 * cntfrq_el0;
    asm volatile ("msr cntp_tval_el0, %0" : : "r" (next_timeout));
}
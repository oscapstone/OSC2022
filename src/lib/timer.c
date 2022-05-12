#include "timer.h"

void core_timer_enable () {

    asm volatile ("mov x0, 1");
    asm volatile ("msr cntp_ctl_el0, x0");
    asm volatile ("mrs x0, cntfrq_el0");
    asm volatile ("lsl x0, x0, 1");
    asm volatile ("msr cntp_tval_el0, x0");
    asm volatile ("mov x0, 2");
    asm volatile ("ldr x1, =0x40000040");
    asm volatile ("str w0, [x1]"); // unmask timer interrupt

    return;
}

unsigned long get_time () {

    unsigned long cntpct_el0;
    unsigned long cntfrq_el0;
    unsigned long time_in_sec;

    asm volatile("mrs %0,  cntpct_el0" : "=r"(cntpct_el0) : );
    asm volatile("mrs %0,  cntfrq_el0" : "=r"(cntfrq_el0) : );

    time_in_sec = cntpct_el0 / cntfrq_el0;

    return time_in_sec;
}

void set_video_timer()
{
    asm volatile ("mov x0, 1");
    asm volatile ("msr cntp_ctl_el0, x0"); // enable
    asm volatile ("mrs x0, cntfrq_el0");
    asm volatile ("lsr x0, x0, 5"); //Set the expired time as core timer frequency shift right 5 bits.
    asm volatile ("msr cntp_tval_el0, x0"); // set expired time
    asm volatile ("mov x0, 2");
    asm volatile ("ldr x1, =0x40000040");
    asm volatile ("str w0, [x1]"); // unmask timer interrupt
}
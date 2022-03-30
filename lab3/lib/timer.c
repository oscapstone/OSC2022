#include "timer.h"

void core_timer_enable() {

    asm volatile(
        "mov x0, 1\n\t"
        "msr cntp_ctl_el0, x0\n\t" // enable
        "mov x0, 2\n\t"
        "ldr x1, =0x40000040\n\t" // CORE0_TIMER_IRQ_CTRL
        "str w0, [x1]\n\t" // unmask timer interrupt
        "ret\n\t"
    );

}
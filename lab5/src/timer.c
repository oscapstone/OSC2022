#include "utils.h"
#include "printf.h"
#include "sched.h"

void handle_timer_irq(void) {
    unsigned long cntfrq_el0;
    asm volatile("mrs %0, cntfrq_el0"
                 : "=r"(cntfrq_el0));
    asm volatile("lsr %0, %0, #5"
                 : "=r"(cntfrq_el0)
                 : "r"(cntfrq_el0));
    asm volatile("msr cntp_tval_el0, %0"
                 :
                 : "r"(cntfrq_el0));
    timer_tick();
}

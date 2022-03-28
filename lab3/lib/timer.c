#include "timer.h"
#include "printf.h"

void core_timer_enable(void){
  unsigned long long frq;
  __asm__ __volatile__(
    "mov x1, 1\n\t"
    "msr cntp_ctl_el0, x1\n\t" // enable
    "mrs x0, cntfrq_el0\n\t"
    "msr cntp_tval_el0, x0\n\t" // set expired time
    "mov x2, 2\n\t"
    "ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n\t"
    "str w2, [x1]\n\t" // unmask timer interrupt
  );
}

#include "timer.h"
#include "mini_uart.h"
#include "sysreg.h"


void core_timer_enable() {
    /*
        cntpct_el0 >= cntp_cval_el0 -> interrupt
        cntp_tval_el0 = cntpct_el0 - cntp_cval_el0
    */
    write_sysreg(cntp_ctl_el0, 1); // enable
    unsigned long frq = read_sysreg(cntfrq_el0);
    write_sysreg(cntp_tval_el0, frq * 2); // set expired time
    *CORE0_TIMER_IRQ_CTRL = 2;            // unmask timer interrupt
}

void core_timer_disable() {
    write_sysreg(cntp_ctl_el0, 0); // disable
    *CORE0_TIMER_IRQ_CTRL = 0;     // unmask timer interrupt
}


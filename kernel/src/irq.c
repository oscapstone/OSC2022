#include <irq.h>


void enable_timer_irq(){
    asm volatile(
        "msr cntp_ctl_el0, %0\n\t"
        ::"r"(1)
    );
    *CORE0_TIMER_IRQ_CTRL |= 2;
}

void disable_timer_irq(){
    asm volatile(
        "msr cntp_ctl_el0, %0\n\t"
        ::"r"(0)
    );
    *CORE0_TIMER_IRQ_CTRL &= ~2;
}

void reset_timer_irq(unsigned long long expired_time){
    asm volatile(
        "msr cntp_cval_el0, %0\n\t"
        ::"r"(expired_time)
    );
}

void set_long_timer_irq(){
     asm volatile(
        "mov x1, 0xfffffffffffffff\n\t" // set a large second
        "msr cntp_cval_el0, x1\n\t" 
        :::"x1"
    );
}

void set_period_timer_irq(){
      asm volatile(
        "mrs x1, cntfrq_el0\n\t"
        "msr cntp_tval_el0, x1\n\t" 
        :::"x1"
    );
}

void enable_irq(){
    asm volatile("msr DAIFClr, 0xf");
}

void disable_irq(){
    asm volatile("msr DAIFSet, 0xf");
}


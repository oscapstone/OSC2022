#include <irq.h>


void enable_timer_irq(){
    asm volatile(
        "msr cntp_ctl_el0, %0\n\t"
        :: "r"(1)
    );
    *CORE0_TIMER_IRQ_CTRL |= 2;
}

void disable_timer_irq(){
    asm volatile(
        "msr cntp_ctl_el0, %0\n\t"
        :: "r"(0)
    );
    *CORE0_TIMER_IRQ_CTRL &= ~2;
}

void enable_irq(){
    asm volatile("msr DAIFClr, 0xf");
}

void disable_irq(){
    asm volatile("msr DAIFSet, 0xf");
}
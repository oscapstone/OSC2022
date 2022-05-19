#include "types.h"
#include "peripherals/iomapping.h"
#include "kernel/timer.h"
#include "debug/debug.h"
#include "asm.h"
static uint64_t jiffies = 0;

/*
 * We use core timer to update jiffies
 */
void inline enable_core_timer_irq(){
    IO_MMIO_write32(CORE0_TIMER_IRQ_CTRL, 2);
}
void inline disable_core_timer_irq(){
    IO_MMIO_write32(CORE0_TIMER_IRQ_CTRL, 0);
}

void init_core_timer(){
    uint64_t freq;

    set_CNTP_CTL_EL0(1);
    freq = get_CNTFRQ_EL0();
    set_CNTP_TVAL_EL0(freq / HZ);
    enable_core_timer_irq();
}
void core_timer_irq_handler(){
    uint64_t freq;
    disable_core_timer_irq();

    freq = get_CNTFRQ_EL0();
    set_CNTP_TVAL_EL0(freq / HZ);

    jiffies += 1;
    enable_core_timer_irq();
}

uint64_t inline get_jiffies(){
    return jiffies;
}


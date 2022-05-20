#include "kernel/timer.h"

static uint64_t jiffies = 0;
static struct list_head timer_list;

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

void init_timer_list(void){
    INIT_LIST_HEAD(&timer_list);
}

// unit of duration is ms
void add_timer(timer_callback callback, uint8_t* data, uint64_t duration){
   timer_t* t = (timer_t*)simple_malloc(sizeof(timer_t)); 
   t->ticks = duration / (1000 / HZ);
   t->callback = callback;
   t->data = data;
   list_add(&t->list, &timer_list);
}



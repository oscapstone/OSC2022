#include "kernel/timer.h"

static uint64_t jiffies = 0;
static struct list_head timer_list;

/*
 * We use core timer to update jiffies
 */
void enable_core_timer_irq(){
    IO_MMIO_write32(CORE0_TIMER_IRQ_CTRL, 2);
}
void disable_core_timer_irq(){
    IO_MMIO_write32(CORE0_TIMER_IRQ_CTRL, 0);
}

void timer_softirq_callback(){
    struct list_head *head = &timer_list;
    struct list_head *node;
    timer_t* t;

    list_for_each(node, head){
        t = list_entry(node, timer_t, list);
        t->ticks--;
        if(t->ticks <= 0){
            list_del(node);
            t->callback(t->data); 
        }
    }
}

void init_timer_list(void){
    INIT_LIST_HEAD(&timer_list);
}

/**
 *  Timer with 10 microsecond resolution
 *
 *  @param duration in microsecond
 */
void add_timer(timer_callback callback, uint8_t* data, uint64_t duration){
    if(duration == 0){
        callback(data);
        return;
    }

    timer_t* t = (timer_t*)simple_malloc(sizeof(timer_t)); 

    t->ticks = duration / (1000 / HZ);
    t->callback = callback;
    t->data = data;
    
    disable_core_timer_irq();
    //critical section
    list_add(&t->list, &timer_list);
    enable_core_timer_irq();
}

void init_core_timer(){
    uint64_t freq;

    init_timer_list();
    set_CNTP_CTL_EL0(1);
    freq = get_CNTFRQ_EL0();
    set_CNTP_TVAL_EL0(freq / HZ);


    // lab5,  user space want to access cpu timer register
    uint64_t tmp;
    asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));

    enable_core_timer_irq();
}
void core_timer_irq_handler(){
    uint64_t freq;
    struct task_struct* current = get_current();

    freq = get_CNTFRQ_EL0();
    set_CNTP_TVAL_EL0(freq / HZ);

    jiffies += 1;
    
    // update task schedule info
    if(current){
        current->sched_info.rticks++;
        current->sched_info.counter--;
        if(current->sched_info.counter <= 0){
            need_sched = 1;
        }
    }
}

uint64_t inline get_jiffies(){
    return jiffies;
}





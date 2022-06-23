#include "timer.h"

// head has nothing, store timer_event_t after it
struct list_head *timer_event_list;

void timer_list_init() {
    unsigned long long tmp;
    asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0" :: "r"(tmp));
    
    timer_event_list = kmalloc(sizeof(list_head_t));
    INIT_LIST_HEAD(timer_event_list);
}

void core_timer_enable() {
    asm volatile(
        "mov x1, 1\n\t"
        "msr cntp_ctl_el0, x1\n\t"  // enable
        "mov x2, 2\n\t"
        "ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n\t"
        "str w2, [x1]\n\t"          // unmask timer interrupt
    );
}

void core_timer_disable() {
    asm volatile(
        "mov x2, 0\n\t"
        "ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n\t"
        "str w2, [x1]\n\t"          // mask timer interrupt
    );
}

void core_timer_handler() {
    lock();
    // if there is no timer_event => set large tval
    if (list_empty(timer_event_list)) {
        set_core_timer_interrupt_abs(get_tick_plus_s(10000));
        unlock();
        return;
    }
    // next timer_event
    timer_event_callback((timer_event_t *)timer_event_list->next);
    unlock();
}

void add_timer(void *function, unsigned long long timeout, char *args, int bytick) {
    timer_event_t *tmp_timer_event = kmalloc(sizeof(timer_event_t));
    // store function into timer_event
    tmp_timer_event->func = function;
    // store argument string into timer_event
    tmp_timer_event->args = kmalloc(strlen(args) + 1);
    memcpy(tmp_timer_event->args, args, strlen(args) + 1);
    // store interrupt time into timer_event
    if (bytick == 0)
        tmp_timer_event->interrupt_time = get_tick_plus_s(timeout);
    else
        tmp_timer_event->interrupt_time = get_tick_plus_s(0) + timeout;

    // add timer_event into timer_event_list (sorted)
    lock();
    struct list_head *curr;
    list_for_each (curr, timer_event_list) {
        // put this timer before the first element larger than it
        if (((timer_event_t *)curr)->interrupt_time > tmp_timer_event->interrupt_time) {
            list_add(&tmp_timer_event->listhead, curr->prev);
            break;
        }
    }

    // if it is the largest
    if (list_is_head(curr, timer_event_list)) {
        list_add_tail(&tmp_timer_event->listhead, timer_event_list);
    }

    // set interrupt to first event
    set_core_timer_interrupt_abs(((timer_event_t *)timer_event_list->next)->interrupt_time);
    unlock();
}

// set timer interrupt (absolutely) use cval
void set_core_timer_interrupt_abs(unsigned long long tick) {
    asm volatile("msr cntp_cval_el0, %0\n\t" :: "r"(tick));
}

// set timer interrupt (relatively) use tval
void set_core_timer_interrupt_rel(unsigned long long expired_time) {
    asm volatile(
        "mrs x1, cntfrq_el0\n\t"
        "mul x1, x1, %0\n\t"
        "msr cntp_tval_el0, x1\n\t"
        :: "r"(expired_time)
    );
}

// get CPU tick add plus seconds
unsigned long long get_tick_plus_s(unsigned long long second) {
    unsigned long long cntpct_el0 = 0, cntfrq_el0 = 0;
    // get current tick
    asm volatile("mrs %0, cntpct_el0\n\t" : "=r"(cntpct_el0));
    // get frequency
    asm volatile("mrs %0, cntfrq_el0\n\t" : "=r"(cntfrq_el0));

    return (cntpct_el0 + cntfrq_el0 * second);
}

// execute timer_event function and set next timer interrupt
void timer_event_callback(timer_event_t *timer_event) {
    // call the callback function in timer_event
    ((void (*)(char *))timer_event->func)(timer_event->args);
    // delete the event
    list_del_entry((struct list_head *)timer_event);
    // free the timer event and its args
    kfree(timer_event->args);
    kfree(timer_event);

    // if next timer_event is existing => set interrupt
    if (!list_empty(timer_event_list))
        set_core_timer_interrupt_abs(((timer_event_t *)timer_event_list->next)->interrupt_time);
    // if there is no timer_event => set large tval
    else
        set_core_timer_interrupt_abs(get_tick_plus_s(10000));
}

void two_second_alert(char *str) {
    unsigned long long cntpct_el0, cntfrq_el0;
    // get current tick
    asm volatile("mrs %0, cntpct_el0\n\t" : "=r"(cntpct_el0));
    // get frequency
    asm volatile("mrs %0, cntfrq_el0\n\t" : "=r"(cntfrq_el0));
    // print message
    uart_printf("Core Timer Interrupt -> seconds after booting : %d\r\n", cntpct_el0 / cntfrq_el0);
    // set next timer
    add_timer(two_second_alert, 2, "", 0);
}

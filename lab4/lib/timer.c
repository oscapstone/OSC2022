#include "timer.h"
#include "utils.h"
#include "memory.h"
#include "mini_uart.h"
#include "mailbox.h"
#include "exception.h"
#include "peripherals/exception.h"

static struct timer_cb *timer_list = 0;

void core_timer_enable() {

    asm volatile(
        "mov x0, 1\n\t"
        "msr cntp_ctl_el0, x0\n\t" // enable
        "mov x0, 2\n\t"
        "ldr x1, =0x40000040\n\t" // CORE0_TIMER_IRQ_CTRL
        "str w0, [x1]\n\t" // unmask timer interrupt
    );

}

void core_timer_disable() {

    asm volatile(
        "mov x0, 0\n\t"
        "ldr x1, =0x40000040\n\t"
        "str w0, [x1]\n\t"
    );

}

void set_timer(unsigned int rel_time) {

    asm volatile(
        "msr cntp_tval_el0, %0\n\t"
        :
        : "r" (rel_time)
    );

}

unsigned int read_timer() {

    unsigned int time;
    asm volatile("mrs %0, cntpct_el0\n\t" : "=r" (time) :  : "memory");
    return time;

}

unsigned int read_freq() {

    unsigned int freq;
    asm volatile("mrs %0, cntfrq_el0\n\t": "=r" (freq) : : "memory");
    return freq;

}
 
void add_timer(void (*func)(void*), void* arg, unsigned int time) {
    struct timer_cb *timer = (struct timer_cb*)simple_malloc(sizeof(struct timer_cb));
    timer->expire_time = ((unsigned int)get32(CNTPCT_EL0)) + time;
    timer->timer_callback = func;
    timer->arg = arg;
    timer->next = 0;
    int update = 0;
    if (!timer_list) {
        timer_list = timer;
        update = 1;
    }
    else if (timer_list->expire_time > timer->expire_time) {
        timer->next = timer_list;
        timer_list = timer;
        update = 1;
    }
    else {
        struct timer_cb *prev = timer_list, *next = timer_list->next;
        while (next && prev->expire_time < timer->expire_time) {
            prev = next;
            next = next->next;
        }
        prev->next = timer;
        timer->next = next;
    }
    
    if (update)
        set_timer(time); // set next interrupt

}

void pop_timer() {

    struct timer_cb *timer = timer_list;
    timer_list = timer_list->next;
    timer->timer_callback(timer->arg);
    if (!timer_list) {
        core_timer_disable();
    }
    else {
        unsigned int cur_time = read_timer();
        set_timer(timer_list->expire_time-cur_time); // set next interrupt
    }

}

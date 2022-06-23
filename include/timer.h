#ifndef TIMER_H
#define TIMER_H
#include "uart.h"
#include "list.h"
#include "string.h"
#include "malloc.h"
#include "simple_alloc.h"

// https://github.com/Tekki/raspberrypi-documentation/blob/master/hardware/raspberrypi/bcm2836/QA7_rev3.4.pdf (p.13)
#define CORE0_TIMER_IRQ_CTRL 0x40000040
#define STR(x) #x
#define XSTR(s) STR(s)

typedef struct timer_event {
    struct list_head listhead;
    // tick time after CPU start (absolutely)
    unsigned long long interrupt_time;
    // interrupt -> func(args)
    void *func;
    char *args;
}timer_event_t;

void timer_list_init();
void core_timer_enable();
void core_timer_disable();
void core_timer_handler();

void add_timer(void *function, unsigned long long timeout, char *args, int bytick);
void set_core_timer_interrupt_abs(unsigned long long tick);
void set_core_timer_interrupt_rel(unsigned long long expired_time);
unsigned long long get_tick_plus_s(unsigned long long second);

void timer_event_callback(timer_event_t *timer_event);
void two_second_alert(char *str);

#endif
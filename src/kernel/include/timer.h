#ifndef _DEF_TIMER
#define _DEF_TIMER

#include <stdint.h>

void coretimer_el0_enable();
void coretimer_el0_handler();
void coretimer_el0_set(uint32_t t); // ms

#define CORE0_TIMER_IRQ_CTRL 0x40000040

struct TIMER_{
    uint64_t time;
    void (*func)(void *);
    void *arg;
    struct TIMER_* next;
};
typedef struct TIMER_ TIMER;
void add_timer(uint64_t time_wait, void (*func)(void *), void *arg);

#endif
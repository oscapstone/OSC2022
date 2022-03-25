#ifndef __TIMER_H
#define __TIMER_H
#define CORE0_TIMER_IRQ_CTRL ((volatile unsigned int *)(0x40000040))

typedef struct {
    const char *message;
} timer_data;

typedef void (*timer_call_back)(timer_data *);

void core_timer_enable();
void core_timer_handler();
void add_timer(unsigned int time, timer_call_back, timer_data *);

#endif

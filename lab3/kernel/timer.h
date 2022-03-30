#ifndef TIMER_H
#define TIMER_H

extern void core_timer_enable();
extern void reset_timer(unsigned int tv_cycle);
unsigned long get_current_time();
void add_timer(void (*callback)(void*), void* data, unsigned int after);
unsigned long get_cpu_freq();
void handle_timer_interrupt();

#endif

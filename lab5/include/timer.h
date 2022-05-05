#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>

uint64_t get_cpu_freq();
uint64_t get_current_time();
void enable_core_timer();
void set_relative_timer(uint32_t tv_cycle);
void set_abs_timer(uint64_t cval);

#endif

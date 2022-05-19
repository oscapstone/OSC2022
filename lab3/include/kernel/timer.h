#ifndef _TIMER_H_
#define _TIMER_H_
#include "types.h"
#define HZ 100
void core_timer_irq_handler();
void init_core_timer();
uint64_t get_jiffies();
void enable_core_timer_irq();
void disable_core_timer_irq();
#endif

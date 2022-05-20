#ifndef _TIMER_H_
#define _TIMER_H_
#include "types.h"
#include "utils.h"
#define HZ 100

typedef void (*timer_callback)(void*);

typedef struct{
    uint64_t ticks;
    timer_callback callback;
    uint8_t *data;
    struct list_head list;
}timer_t;

extern void core_timer_irq_handler();
extern void init_core_timer();
extern uint64_t get_jiffies();
extern void enable_core_timer_irq();
extern void disable_core_timer_irq();
extern void init_timer_list(); 
extern void add_timer(timer_callback, uint8_t*, uint64_t);

#endif

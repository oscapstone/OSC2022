#ifndef TASK_H
#define TASK_H

#include "list.h"

// set uart irq priority = 1, timer = 0
#define UART_IRQ_PRIORITY 1  
#define TIMER_IRQ_PRIORITY 0

//like timer_event 
typedef struct task
{
    struct list_head listhead;

    unsigned long long priority; //store priority (smaller number is more preemptive)

    void *task_function; // task function pointer
} task_t;

void add_task(void *task_function, unsigned long long priority);
void run_task(task_t *the_task);
void run_preemptive_tasks();
void task_list_init();

#endif
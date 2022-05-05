#ifndef TASK_H
#define TASK_H
#include "list.h"
#include "malloc.h"
#include "exception.h"
#include "simple_alloc.h"

typedef struct task {
    struct list_head listhead;
    // store priority
    unsigned long long priority;
    // task function pointer
    void *func;
}task_t;

void task_list_init();
void add_task(void *function, unsigned long long priority);
void run_preemptive_tasks();

#endif
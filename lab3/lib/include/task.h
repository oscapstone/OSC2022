#ifndef __TASK_H__
#define __TASK_H__

#include "list.h"
#include "malloc.h"
#include "string.h"

typedef struct task_event {
    struct list_head node;
    void* callback;
} task_event_t;

void add_task();
#endif
#ifndef MALLOC_H
#define MALLOC_H

#include "list.h"
#include "uart.h"
#include "exception.h"
#define MAXORDER 6
// simple_malloc
void *simple_malloc(unsigned int size);

//buddy system
void *kmalloc(unsigned int size);
void kfree(void *ptr);

//like timer_event
typedef struct frame
{
    struct list_head listhead;
    int val; //store priority (smaller number is more preemptive)
    unsigned int idx;
} frame_t;

void init_allocator();
frame_t *release_redundant(frame_t *frame);
frame_t *get_buddy(frame_t *frame);
void dump_list_info();

#endif
#ifndef _DEF_QUEUE
#define _DEF_QUEUE
#include <stdint.h>
#include <lock.h>
typedef struct Queue_Entry_{
    void *entry;
    struct Queue_Entry_ *fd;
} Queue_Entry;

typedef struct Queue_{
    Lock *lock;
    Queue_Entry *front;
    Queue_Entry *back;
    uint32_t size;
    uint32_t flags;
} Queue;

void *queue_front(Queue *qu);
void *queue_pop(Queue *qu);
void queue_push(Queue *qu, void *entry);
Queue *queue_new();
#endif
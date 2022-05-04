#ifndef _DEF_LOCKQUEUE
#define _DEF_LOCKQUEUE
#include <stdint.h>

typedef struct LockQueue_Entry_{
    void *entry;
    struct LockQueue_Entry_ *fd;
} LockQueue_Entry;

typedef struct LockQueue_{
    LockQueue_Entry *front;
    LockQueue_Entry *back;
    uint32_t size;
    uint32_t flags;
} LockQueue;

void *lockqueue_front(LockQueue *qu);
void *lockqueue_pop(LockQueue *qu);
void lockqueue_push(LockQueue *qu, void *entry);
LockQueue *lockqueue_new();
#endif
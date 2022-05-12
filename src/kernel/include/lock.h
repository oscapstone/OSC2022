#ifndef _DEF_LOCK
#define _DEF_LOCK

#include <stdint.h>
#include <lockqueue.h>

typedef struct Lock_{
    uint64_t hold_thread;
    LockQueue *waitqueue;
} Lock;

Lock *lock_new();
void lock_get(Lock *lock);
void lock_release(Lock *lock);

#endif
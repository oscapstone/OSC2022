#include <lock.h>
#include <interrupt.h>
#include <sched.h>
#include <queue.h>
#include <kmalloc.h>

Lock *lock_new()
{
    Lock *lock = (Lock *)kmalloc(sizeof(Lock));
    lock->hold_thread = 0;
    lock->waitqueue = lockqueue_new();
    return lock;
}

void lock_get(Lock *lock)
{
    while(1){
        size_t flags = interrupt_disable_save();
        if(lock->hold_thread == 0){
            lock->hold_thread = thread_get_current();
            interrupt_enable_restore(flags);
            return ;
        }
        interrupt_enable_restore(flags);
        waitlock(lock->waitqueue);
    }
}

void lock_release(Lock *lock)
{
    size_t flags = interrupt_disable_save();
    if(lock->hold_thread!=thread_get_current()){
        interrupt_enable_restore(flags);
        return ;
    }
    lock->hold_thread = 0;
    wakeuplock(lock->waitqueue);
    interrupt_enable_restore(flags);
}
#ifndef _DEF_SCHED
#define _DEF_SCHED

#include <stdint.h>
#include <queue.h>
#include <process.h>
typedef struct CPUState_{
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t fp;
    uint64_t lr;
    uint64_t sp;
    uint64_t sp_el0;
    uint64_t elr_el1;
    uint64_t pad;
} CPUState;

typedef enum ThreadStatus_{
    ThreadStatus_init = 0,
    ThreadStatus_running = 1,
    ThreadStatus_wait = 2,
    ThreadStatus_exit = 3,
    ThreadStatus_zombie = 4,
} ThreadStatus;

typedef struct Thread_{
    uint64_t thread_id;
    // uint64_t process_id;
    ThreadStatus status;
    void *arg;
    struct Thread_* fd;
    Process *process;
    void (*func)(void *);
    void *stack_pointer;
    uint64_t stack_size;
    CPUState saved_reg; //10: fp, 11: lr, 12: sp
} Thread;

void schedule();
extern uint64_t thread_get_current();
Thread* get_thread(uint64_t tid);
Thread* create_thread(void (*func)(void *), void* arg);
void thread_start(void (*func)(void *));
void wait(Queue *waitqueue);
void waitlock(LockQueue *waitqueue);
void wait_thread(Thread *thread);
void wakeup(Queue *waitqueue);
void wakeuplock(LockQueue *waitqueue);
void wakeup_thread(Thread *thread);
void sched_preempt();
void thread_run(Thread *thread);
void sched_init();
#endif
#ifndef THREAD_H 
#define THREAD_H

#include "type.h"
#include "list.h"
#include "signal.h"

#define PIDMAX 16
#define THREAD_STACK_SIZE 0x10000

extern void switch_to(void *curr_context, void *next_context);
extern void read_context(void *context);
extern void write_context(void *context);
extern void *get_current();

typedef struct threadContext {
    uint64 x19;
    uint64 x20;
    uint64 x21;
    uint64 x22;
    uint64 x23;
    uint64 x24;
    uint64 x25;
    uint64 x26;
    uint64 x27;
    uint64 x28;
    uint64 fp;
    uint64 lr;
    uint64 sp;
} threadContext_t;

typedef struct thread {
    list_head_t listHead;
    threadContext_t context;
    char *data;
    uint64 datasize;
    int pid;
    state_t state;
    char *stackPtr;
    char *kernel_stackPtr;
    void (*signal_handlers[SIGMAX + 1])();
    signal_pair_t signal_pair;
    bool has_signal;
    threadContext_t signal_context;
} thread_t;



extern thread_t *currThread;
extern thread_t threads[PIDMAX + 1];
extern list_head_t *run_queue, *wait_queue;

void initThreads();
thread_t *createThread(void *program);
void idle();
void schedule();
void execThread(char *program, uint64 program_size);
void testThread();
void thread_exit();
void set_schedule_timer();
void schedule_callback(char *message);
#endif

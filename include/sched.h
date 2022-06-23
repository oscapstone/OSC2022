#ifndef SCHED_H
#define SCHED_H
#include "vfs.h"
#include "list.h"
#include "buddy.h"
#include "signal.h"
#include "malloc.h"
#include "syscall.h"
#include "exception.h"
#include "simple_alloc.h"

#define PIDMAX 4096
#define USTACK_SIZE 0x10000
#define KSTACK_SIZE 0x10000
#define SIGNAL_MAX 16

extern void switch_to(void *curr_context, void *next_context);
extern void store_context(void *curr_context);
extern void load_context(void *curr_context);
extern void *get_current();

typedef struct thread_context {
    unsigned long x19, x20, x21, x22, x23, x24, x25, x26, x27, x28;
    unsigned long fp, lr, sp;
} thread_context_t;

typedef struct thread {
    list_head_t listhead;
    thread_context_t context;
    char *data;
    unsigned int datasize;
    int iszombie;
    int pid;
    int isused;
    char* stack_alloced_ptr;
    char* kernel_stack_alloced_ptr;
    void (*singal_handler[SIGNAL_MAX+1])();
    int sigcount[SIGNAL_MAX + 1];
    void (*curr_signal_handler)();
    int signal_is_checking;
    thread_context_t signal_saved_context;
    char curr_working_dir[256];
    struct file* file_descriptors_table[16];
} thread_t;

extern thread_t *curr_thread;
extern list_head_t *run_queue;
extern list_head_t *wait_queue;
extern thread_t threads[PIDMAX + 1];

void init_thread_sched();
void idle();
void schedule();
void kill_zombies();
thread_t *thread_create(void *start);
void thread_exit();
int exec_thread(char *data, unsigned int filesize);
void schedule_timer(char *notuse);

#endif

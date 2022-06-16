#ifndef __THREAD_H__
#define __THREAD_H__

#include "exception.h"
#include "mm.h"

typedef int pid_t;

#define THREAD_RUNNABLE     0
#define THREAD_RUNNING      1
#define THREAD_WAIT         2
#define THREAD_EXIT         3
#define THREAD_STACK_SIZE   (PAGE_SIZE * 4)
#define THREAD_MAX_SIG_NUM  16

struct thread_context {
    unsigned long x19;
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long fp;
    unsigned long lr;
    unsigned long sp;
};

struct thread {
    pid_t pid;
    struct thread *parent;
    int child_num;

    unsigned long code_addr;
    unsigned long u_stack;
    unsigned long k_stack;
    unsigned int status;
    unsigned long ksp;
    int privilege;

    struct thread_context context;
    // For POSIX Signal
    struct thread_context sig_context;

    void (*sig_handlers[THREAD_MAX_SIG_NUM]) ();
    unsigned int sig_num[THREAD_MAX_SIG_NUM];
    int under_sig;

    struct thread *next;
};

struct thread_queue {
    struct thread *head;
    struct thread *tail;
};

struct thread_queue *runnable_queue;

#define PID_NUM 16
pid_t pid_used[PID_NUM];

int THREAD_START;


struct thread* thread_dequeue(struct thread_queue* queue);
void thread_enqueue(struct thread_queue* queue, struct thread* thread);
void thread_module_init();
pid_t get_new_pid();
struct thread* thread_create(void (*func) ());
struct thread* get_cur_thread();
pid_t get_cur_thread_id();
void thread_schedule();
void idle_thread();
void thread_exit();
void thread_exec (void (*prog)());
pid_t thread_fork(struct trap_frame* trap_frame);
void thread_kill(pid_t pid);
void default_sig_handler();
void check_signal();
void *get_sig_handler(struct thread* thread);
void thread_sig_register(int sig, void (*handler)());
void thread_sig_kill(int pid, int sig);
void thread_sig_return();
#endif
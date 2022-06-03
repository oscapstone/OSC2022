#ifndef _SCHEDULE_HEADER_
#define _SCHEDULE_HEADER_
#define THREAD_STACK_SIZE 4096//4096
#include "signal.h"
#include "vfs.h"
#include "tmpfs.h"
enum task_state
{
    EMPTY,
    RUNNABLE,
    WAITING,
    RUNNING,
    FINISHED,
    ZOMBIE,
};
typedef struct cpu_context
{
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
}cpu_context;
typedef struct Thread_struct
{
    cpu_context cpu_context;
    enum task_state state;
    unsigned int id;
    void* user_stack;
    void* kernel_stack;
    struct Thread_struct *next,*prev;
    // void (*singal_handler[10])();
    void (*signal_handler[20])();
    unsigned int signal_count[20];
    void* signal_ustack;
    void (*run_handler)();
    cpu_context saved_context;

    struct file* fd_table[20];
    struct vnode* pwd;

}Thread_struct;
typedef struct task_queue
{
    Thread_struct* task;
    struct task_queue *next,*prev;
}task_queue;

int thread_create(void (*f)());
Thread_struct current_thread();
void schedule();
void idle();
void kill_zombie();
void init_sched();
void push_thread(Thread_struct*);
void iter_runqueue();
void exec_thread();
void context_switch(Thread_struct* next);
Thread_struct* pop_thread();
void thread_exit();
void thread_exec();
Thread_struct* get_thread(int);
void kill_thread(int);
extern Thread_struct* get_current();
void raise_signal(int pid,int signal);
#endif
#ifndef _TASK_H
#define _TASK_H

#include <stddef.h>

#define TERMINATED 0
#define RUNNING 1
#define WAITING 2

typedef struct {
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
    unsigned long fp; //x29: frame pointer
    unsigned long lr; //x30: link register for function calls
    unsigned long sp;
} cpu_context;

typedef struct task_struct {
    cpu_context context;
    int id;
    int state;
    struct task_struct *prev;
    struct task_struct *next;
} task_struct;

typedef struct task_queue {
    char name[10];
    task_struct *begin;
    task_struct *end;
} task_queue;

void run_main_thread();
task_struct* thread_create(void* func);
void thread_schedule();
void kill_zombies();
void idle();
int getpid();

void dump_queue(task_queue *queue);
void push_task_to_queue(task_queue *queue, task_struct *task);
void pop_task_from_queue(task_queue *queue, task_struct *task);

#endif
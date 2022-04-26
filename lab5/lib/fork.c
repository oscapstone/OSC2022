#include "fork.h"
#include "mm.h"
#include "../include/sched.h"
#include "../include/entry.h"

int copy_process(unsigned long fn, unsigned long arg) {
    // why disable preemption?
    preempt_disable();
    struct task_struct *p;

    p = (struct task_struct *) malloc(PAGE_SIZE);
    if (p == NULL)
        return 1;
    p->priority = current->priority;
    p->state = TASK_RUNNING;
    p->counter = p->priority;
    p->preempt_count = 1;
    
    p->cpu_context.x19 = fn;
    p->cpu_context.x20 = arg;
    p->cpu_context.pc = (unsigned long)ret_from_fork;
    p->cpu_context.sp = (unsigned long)p + THREAD_SIZE;

    int pid = nr_tasks++;
    task[pid] = p;
    p->id = pid;
    preempt_enable();

    return 0;
}
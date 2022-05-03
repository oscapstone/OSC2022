#include "sched.h"
#include "entry.h"
#include "buddy_system.h"
#include "printf.h"
#include "stdlib.h"

int thread_create(void *fn) {
    preempt_disable();
    struct task_struct *p;

    p = (struct task_struct *)malloc(sizeof(struct task_struct));
    if (!p)
        return 1;
    p->priority = current->priority;
    p->state = TASK_RUNNING;
    p->counter = p->priority;
    p->preempt_count = 1; // disable preemtion until schedule_tail

    p->cpu_context.x19 = (unsigned long)fn;
    p->cpu_context.pc = (unsigned long)run_thread;
    p->kernel_sp = (unsigned long)buddy_system_alloc(4) + 4096 - 16;
    p->user_sp = (unsigned long)buddy_system_alloc(4) + 4096 - 16;
    p->cpu_context.sp = p->kernel_sp; // kernel space

    int pid = get_new_pid();
    task[pid] = p;
    p->pid = pid;
    preempt_enable();
    return pid;
}

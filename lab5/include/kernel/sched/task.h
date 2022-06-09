#ifndef _TASK_H_
#define _TASK_H_

#include "kernel/sched/sched.h"
#include "fs/initrdfs.h"
#include "mm/page_alloc.h"
#include "debug/debug.h"
#include "mm/slab.h"

extern void task_init();
extern uint64_t task_create();
extern void task_exit();
extern void task_destroy(struct task_struct*);
extern void run_init_task(char*);
extern struct trap_frame* get_current_trap_frame();

#endif

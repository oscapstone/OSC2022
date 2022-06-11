#ifndef _KTHREAD_H_
#define _KTHREAD_H_

#include "kernel/sched/sched.h"
#include "mm/page_alloc.h"
#include "debug/debug.h"
#include "mm/slab.h"
typedef void (*kthread_func)(void);

extern void kthread_init();
extern void kthread_idle();
extern void kthread_test();
extern uint64_t kthread_create(kthread_func func);
extern void kthread_exit();
extern void kthread_destroy(struct task_struct*);
extern void kthread_start(kthread_func);

#endif

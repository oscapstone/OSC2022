#ifndef _SCHED_H_
#define _SCHED_H_
#include "mm/mmu.h"
#include "lib/list.h"
#include "kernel/irq_handler.h"
#include "kernel/signal.h"

typedef uint64_t pid_t;


#define TASK_RUNNING        0
#define TASK_INTERRUPTIBLE  1
#define TASK_UNINTERRUPTIBLE    2
#define TASK_DEAD		64
#define VMA_STACK 1
#define VMA_PROGRAM 2
#define VMA_VC_RAM 3 

#define VMA_CODE_BASE 0
#define VMA_STACK_END 0xfffffffff000
#define VMA_STACK_SIZE 0x4000
#define VMA_VC_BASE 0x3c000000
#define VMA_VC_END  0x3f000000

#define get_trap_frame(task) (task->stack + PAGE_SIZE * 2 - sizeof(struct trap_frame))
struct thread_info{
    pid_t pid;
    volatile int64_t state;
};

struct sched_info{
    int64_t rticks; // running ticks = user + system ticks
    int64_t priority;
    int64_t counter; // reset counter -> counter = priority
    struct list_head sched_list; 
};

struct trap_frame{
    uint64_t x0;
    uint64_t x1;
    uint64_t x2;
    uint64_t x3;
    uint64_t x4;
    uint64_t x5;
    uint64_t x6;
    uint64_t x7;
    uint64_t x8;
    uint64_t x9;
    uint64_t x10;
    uint64_t x11;
    uint64_t x12;
    uint64_t x13;
    uint64_t x14;
    uint64_t x15;
    uint64_t x16;
    uint64_t x17;
    uint64_t x18;
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
    uint64_t x29; // fp
    uint64_t x30; // lr
    uint64_t sp_el0;
    uint64_t spsr_el1;
    uint64_t elr_el1;
}__attribute__((packed));

struct task_ctx{
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
    uint64_t redzone;
}__attribute__((packed));

struct vm_area_struct{
    uint64_t vm_start;
    uint64_t vm_end;
    uint64_t type;
    struct list_head list;
};

struct mm_struct{
    struct list_head mmap_list;
	pgdval_t* pgd;
};

struct task_struct{
    struct task_ctx ctx;

    struct thread_info thread_info;

    struct mm_struct* mm;

    // kernel stack
    void* stack;

    // schedule info
    struct sched_info sched_info;

    // signal
    struct sigpending sigpending;
    struct sighand_struct sighandler;

    // relationship
    struct task_struct* parent;
    struct task_struct* child;
    struct list_head siblings;

    // task list
    struct list_head list;

    // zombie list
    struct list_head zombie;
};

extern int need_sched;
extern struct list_head zombies;
extern struct list_head task_list;
extern struct task_struct* user_init;

extern void add_task_to_rq(struct task_struct *task);
extern struct task_struct* pick_next_task_from_rq();
extern void schedule();
extern void switch_to(struct task_struct*, struct task_struct*);
extern struct task_struct* get_current();
extern pid_t get_pid_counter(void);
extern void preempt_schedule();
extern void print_rq(void);
extern struct task_struct* find_task_by_pid(uint64_t pid);
#endif

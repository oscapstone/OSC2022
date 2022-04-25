#include "task.h"
#include "allocator.h"
#include "switch.h"
#include "shell.h"
#include "utils.h"
#include "mini_uart.h"
#include "sysreg.h"
#include "cpio.h"


task_queue run_queue = {"run", NULL, NULL};
task_queue wait_queue = {"wait", NULL, NULL};
task_queue terminated_queue = {"terminated", NULL, NULL};
static int task_cnt = 0;

void foo() {
    for(int i = 0; i < 10; ++i) {
        uart_printf("Thread id: %d %d\n", get_current()->id, i);
        delay(1000000);
        thread_schedule();
    }
}

void run_user_program() {
    debug_mode = 0;
    load_program("user.img");
    //core_timer_enable();
    task_struct* root_task = get_current();
    asm volatile("mov x0, 0x340   \n"::);
    asm volatile("msr spsr_el1, x0   \n"::);
    asm volatile("msr elr_el1,  %0   \n"::"r"(USER_PROGRAM_ADDR));
    asm volatile("msr sp_el0,   %0   \n"::"r"(root_task->user_fp));
    asm volatile("eret  \n"::);
}

void run_main_thread() {
    task_struct* root_task = thread_create(idle);
    write_sysreg(tpidr_el1, root_task);
    thread_create(run_user_program);
    idle();
}

task_struct* thread_create(void *func) {
    task_struct* new_task = (task_struct*)page_malloc(0);
    new_task->context.fp = (unsigned long)new_task + PAGE_SIZE_4K - 1;
    new_task->context.lr = (unsigned long)func;
    new_task->context.sp = (unsigned long)new_task + PAGE_SIZE_4K - 1;  // kernel stack pointer for the thread
    new_task->user_fp = page_malloc(0) + PAGE_SIZE_4K - 1;  // user stack frame pointer

    new_task->state = RUNNING;
    new_task->id = task_cnt++;

    debug_printf("[DEBUG][thread_create] id: %d\n", new_task->id);

    push_task_to_queue(&run_queue, new_task);
    return new_task;
}

/* The state/queue of the current thread should be taken care of before entering this function */
void thread_schedule() {
    task_struct *next_task = run_queue.begin;
    if (!next_task->id && !next_task->next) {
        thread_create(shell);
        thread_schedule();
    }

    pop_task_from_queue(&run_queue, next_task);
    push_task_to_queue(&run_queue, next_task);

    task_struct *cur = get_current();
    debug_printf("[DEBUG][thread_schedule] switch from thread %d to %d\n", cur->id, next_task->id);
    switch_to(cur, next_task);
}

void idle() {
    while(1) {
        kill_zombies();
        thread_schedule();
    }
}

void kill_zombies() {
    
}

void push_task_to_queue(task_queue *queue, task_struct *task) {
    if (queue->begin) {
        queue->end->next = task;
        task->prev = queue->end;
        task->next = NULL;
        queue->end = task;
    }
    else {
        queue->begin = queue->end = task;
        task->prev = task->next = NULL;
    }
    debug_printf("[DEBUG][push_task_to_queue] push thread %d into %s queue\n", task->id, queue->name);
}

void pop_task_from_queue(task_queue *queue, task_struct *task) {
    task_struct *prev = task->prev;
    task_struct *next = task->next;
    if (prev && next) {
        prev->next = next;
        next->prev = prev;
    }
    else if (!prev && next) {
        next->prev = NULL;
        queue->begin = next;
    }
    else if (prev && !next) {
        prev->next = NULL;
        queue->end = prev;
    }
    else
        queue->begin = queue->end = NULL;
    debug_printf("[DEBUG][pop_task_from_queue] pop thread %d from %s queue\n", task->id, queue->name);
}

void dump_queue(task_queue *queue) {
    task_struct *task = queue->begin;
    uart_printf("%s queue: ", queue->name);
    while (task) {
        uart_printf("%d ", task->id, task);
        task = task->next;
    }
    uart_printf("\n");
}
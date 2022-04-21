#include "task.h"
#include "allocator.h"
#include "switch.h"
#include "shell.h"
#include "utils.h"
#include "mini_uart.h"
#include "sysreg.h"


static task_queue run_queue = {"run", NULL, NULL};
static task_queue wait_queue = {"wait", NULL, NULL};
static task_queue terminated_queue = {"terminated", NULL, NULL};
static int task_cnt = 0;
static task_struct main_task;

void foo() {
    for(int i = 0; i < 10; ++i) {
        uart_printf("Thread id: %d %d\n", get_current()->id, i);
        delay(1000000);
        thread_schedule();
    }
}

void run_main_thread() {
    main_task.id = 0;
    write_sysreg(tpidr_el1, &main_task);
    debug_mode = 1;
	for(int i = 0; i < 3; ++i) {
        thread_create(foo);
    }
    idle();
}

task_struct* thread_create(void *func){
    task_struct* new_task = (task_struct*)page_malloc(0);
    new_task->context.fp = (unsigned long)new_task + sizeof(task_struct);
    new_task->context.lr = (unsigned long)func;
    new_task->context.sp = (unsigned long)new_task + PAGE_SIZE_4K - 1;

    new_task->state = RUNNING;
    new_task->id = ++task_cnt;

    debug_printf("[DEBUG][thread_create] id: %d\n", new_task->id);

    push_task_to_queue(&run_queue, new_task);
    return new_task;
}

/* The state/queue of the current thread should be taken care of before entering this function */
void thread_schedule() {
    task_struct *next_task = run_queue.begin;
    if (!next_task)
        idle();

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
    if (queue->end) {
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
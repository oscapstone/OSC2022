#include "task.h"

// init a low priority
int curr_priority = 9999;

struct list_head *task_list;

void task_list_init() {
    INIT_LIST_HEAD(task_list);
}

void add_task(void *function, unsigned long long priority) {
    task_t *tmp_task = simple_alloc(sizeof(task_t));
    // store function into task
    tmp_task->func = function;
    // store priority into task
    tmp_task->priority = priority;

    // init list head
    INIT_LIST_HEAD(&tmp_task->listhead);

    // critical section
    disable_interrupt();

    // add tmp_task into task_list (sorted)
    struct list_head *curr;
    list_for_each(curr, task_list) {
        if (((task_t *)curr)->priority > tmp_task->priority) {
            list_add(&tmp_task->listhead, curr->prev);
            break;
        }
    }

    // if it is the largest
    if (list_is_head(curr, task_list))
        list_add_tail(&tmp_task->listhead, task_list);

    enable_interrupt();
}

void run_preemptive_tasks() {
    // do the tasks with interrupts enabled
    enable_interrupt();

    while (!list_empty(task_list)) {
        // critical section
        disable_interrupt();

        task_t *tmp_task = (task_t *)task_list->next;
        // check for preemption
        if (tmp_task->priority >= curr_priority) {
            enable_interrupt();
            break;
        }
        // verify preemption
        // if (curr_priority != 9999) {
        //     uart_printf("---preemption---\r\n");
        //     uart_printf("task priority : %d, curr priority %d\r\n", tmp_task->priority, curr_priority);
        // }
        list_del_entry((struct list_head *)tmp_task);
        // store previous task priority. if there is not preemption => prev_priority = 9999
        int prev_priority = curr_priority;
        // before running the task, set its priority to curr_priority
        curr_priority = tmp_task->priority;
        enable_interrupt();

        // run task
        ((void (*)())tmp_task->func)();

        // after running the task, set prev_priority equals to curr_priority
        disable_interrupt();
        curr_priority = prev_priority;
        enable_interrupt();
    }
}

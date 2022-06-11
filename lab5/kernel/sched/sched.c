#include "kernel/sched/sched.h"

LIST_HEAD(zombies);
LIST_HEAD(rq);
LIST_HEAD(task_list);
uint64_t pid_count = 0;
int need_sched = 0;
struct task_struct* user_init = NULL;

void add_task_to_rq(struct task_struct *task){
    volatile uint64_t daif;
    daif = local_irq_disable_save();
    task->thread_info.state = TASK_RUNNING;

    list_add_tail(&task->sched_info.sched_list, &rq);
    need_sched = 1;
    local_irq_restore(daif);
}

struct task_struct* pick_next_task_from_rq(){
    if(!list_empty(&rq))
        return list_first_entry(&rq, struct task_struct, sched_info.sched_list);
    else
        return NULL;
}

void schedule(){
    struct task_struct* current, *next;
    // prevent running scheduler in softirq
    if(in_softirq()){
        return;
    }
    
    if(need_sched){
        need_sched = 0;
        next = pick_next_task_from_rq();

        // add current task to schdule list
        current = get_current();
        if( current != NULL && current->thread_info.state != TASK_DEAD){ 
            current->sched_info.counter = current->sched_info.priority;
            list_add_tail(&current->sched_info.sched_list, &rq);
        }

        // context switch
        if(next != NULL){
            list_del(&next->sched_info.sched_list);
            switch_to(current, next);
        }
    }
}

void preempt_schedule(){
    local_irq_disable();
    need_sched = 1;
    schedule();
    local_irq_enable();
}

pid_t get_pid_counter(void){
    pid_t ret;
    ret = pid_count++; 
    return ret;
}

struct task_struct* find_task_by_pid(uint64_t pid){
    struct list_head *node;
    struct task_struct *tmp_task;
    uint64_t daif;

    daif = local_irq_disable_save();
    list_for_each(node, &task_list){
        tmp_task = list_entry(node, struct task_struct, list);
        if(tmp_task->thread_info.pid == pid){
            local_irq_restore(daif);
            return tmp_task;
        }
    }
    local_irq_restore(daif);
    return NULL;
}


/* debug */
void print_rq(void){
    struct list_head *node;
    struct task_struct *tmp_task;
    struct task_pid *tmp_task_pid;
    volatile uint64_t daif;
    // In printf, interrupt will be enable, so we can't directly print run queue
    daif = local_irq_disable_save();
    list_for_each(node, &task_list){
        tmp_task = list_entry(node, struct task_struct, list);
        printf("%l ", tmp_task->thread_info.pid);
    }
    local_irq_restore(daif);
    printf("\r\n");
}

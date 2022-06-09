#include "kernel/sched/sched.h"

uint64_t pid_count = 0;
int need_sched = 0;
LIST_HEAD(rq);
void add_task_to_rq(struct task_struct *task){
    uint64_t daif;
    task->thread_info.state = TASK_RUNNING;

    daif = local_irq_disable_save();
    list_add_tail(&task->sched_info.sched_list, &rq);
    need_sched = 1;
    local_irq_restore(daif);
}
struct task_struct* pick_next_task_from_rq(){
    return list_first_entry(&rq, struct task_struct, sched_info.sched_list);
}

void schedule(){
    struct task_struct* current, *next;
    uint64_t daif;
    // prevent running scheduler in softirq
    if(in_softirq()) return;

    daif = local_irq_disable_save();
    if(need_sched){
        need_sched = 0;
       
        // add current task to schdule list
        current = get_current();
        if( current != NULL && current->thread_info.state != TASK_DEAD){ 
            list_add_tail(&current->sched_info.sched_list, &rq);
        }

        // context switch
        next = pick_next_task_from_rq();
        list_del(&next->sched_info.sched_list);

        switch_to(current, next);
    }
    local_irq_restore(daif);
}

void update_sched_info(struct task_struct* task){
    task->sched_info.rticks++;
    task->sched_info.counter--;
    if(task->sched_info.counter <= 0){
        task->sched_info.counter = task->sched_info.priority;
        need_sched = 1;
    }
}

pid_t get_pid_counter(void){
    pid_t ret;
    uint64_t daif;
    daif = local_irq_disable_save();
    ret = pid_count++; 
    local_irq_restore(daif);
    return ret;
}


/* debug */
struct task_pid{
    pid_t pid;
    struct list_head list;
};
void print_rq(void){
    struct list_head *node;
    struct task_struct *tmp_task;
    struct task_pid *tmp_task_pid;
    uint64_t daif;
    LIST_HEAD(head);
    // In printf, interrupt will be enable, so we can't directly print run queue
    daif = local_irq_disable_save();
    list_for_each(node, &rq){
        tmp_task = list_entry(node, struct task_struct, sched_info.sched_list);
        tmp_task_pid = (struct task_pid*)kmalloc(sizeof(struct task_pid));
        tmp_task_pid->pid = tmp_task->thread_info.pid;
        list_add_tail(&tmp_task_pid->list, &head);
    }
    local_irq_restore(daif);

    while(!list_empty(&head)){
        tmp_task_pid = list_first_entry(&head, struct task_pid, list);
        printf("%l ", tmp_task_pid->pid);
        list_del(head.next);
        kfree(tmp_task_pid);
    }
    printf("\r\n");
}

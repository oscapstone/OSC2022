#include "kernel/sched/kthread.h"

LIST_HEAD(kthread_zombies);

void kthread_destroy(struct task_struct* task){
//    LOG("kthread_destroy %l", task->thread_info.pid);
    free_pages(task->stack, 1);
    kfree(task);
}
void _kthread_remove_zombies(){
    struct list_head* node;
    struct task_struct* zombie;
    //uint64_t daif;
    //daif = local_irq_disable_save();
    while(!list_empty(&kthread_zombies)){
        zombie = list_first_entry(&kthread_zombies, struct task_struct, siblings);
        LOG("Remove %l", zombie->thread_info.pid);
        list_del(kthread_zombies.next); 
        kthread_destroy(zombie);
    }
    //local_irq_restore(daif);
}

void kthread_idle(){
    uint64_t daif;
    uint64_t c = 0;
    while(1){
        daif = local_irq_disable_save();
        _kthread_remove_zombies();
        need_sched = 1;
        printf("%l: sp %l\r\n", c++, get_SP());
        print_rq();
        schedule();
        //printf("sp %l\r\n", get_SP());
        local_irq_restore(daif);
    }
}

void kthread_test(){
    struct task_struct* cur;
    cur = get_current();
    for(uint32_t i = 0 ; i < 10 ; i++){
        printf("%u: pid: %l\r\n",i , cur->thread_info.pid);
        for(uint32_t j = 0 ; j < 100000 ; j ++){
            asm volatile("nop");
        }
    }
    kthread_exit();
}

uint64_t kthread_create(kthread_func func){
    LOG("kthread enter");
    uint64_t daif = local_irq_disable_save();
    struct task_struct* kthread = (struct task_struct*)kmalloc(sizeof(struct task_struct));

    // initialize kernel stack
    kthread->stack = alloc_pages(1);

    // initialize thread_info
    kthread->thread_info.pid = get_pid_counter();
    kthread->thread_info.state = TASK_RUNNING;

    // initialize thread context
    kthread->ctx.lr = (uint64_t)func;
    kthread->ctx.sp = (uint64_t)kthread->stack + PAGE_SIZE * 2;
    kthread->ctx.fp = kthread->ctx.sp;

    // initialize mm
    kthread->mm = NULL;

    // initialize relationship
    kthread->parent = NULL;
    kthread->child = NULL;
    INIT_LIST_HEAD(&kthread->siblings);

    // initialize schedule info
    kthread->sched_info.rticks = 0;
    kthread->sched_info.priority = 1;
    kthread->sched_info.counter = kthread->sched_info.priority;
    
    LOG("kthread end");
    add_task_to_rq(kthread);
    local_irq_restore(daif);
    return 0;
}

void kthread_exit(){
    LOG("kthread_exit start");
    struct task_struct* cur;
    uint64_t daif;
    cur = get_current();
    cur->thread_info.state = TASK_DEAD;

    daif = local_irq_disable_save();
    list_add_tail(&cur->siblings, &kthread_zombies);
    need_sched = 1; 
    local_irq_restore(daif);

    LOG("kthread_exit end");
    schedule();
}

#include "kernel/sched/kthread.h"

extern void kthread_trampoline();

void kthread_destroy(struct task_struct* task){
//    LOG("kthread_destroy %l", task->thread_info.pid);
    list_del(&task->list);
    free_pages(task->stack, 1);
    kfree(task);
}
void _kthread_remove_zombies(){
    struct list_head* node;
    struct task_struct* zombie;
    volatile uint64_t daif;
    //volatile uint64_t daif;
    //daif = local_irq_disable_save();
    local_irq_disable();
    while(!list_empty(&zombies)){
        zombie = list_first_entry(&zombies, struct task_struct, zombie);
        LOG("Remove %l", zombie->thread_info.pid);
        list_del(zombies.next); 
        if(zombie->mm == NULL) kthread_destroy(zombie);
        else task_destroy(zombie);
    }
    local_irq_enable(daif);
    //local_irq_restore(daif);
}

void kthread_idle(){
    while(1){
        preempt_schedule();
        _kthread_remove_zombies();
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
}

uint64_t kthread_create(kthread_func func){
    LOG("kthread enter");
    volatile uint64_t daif;
    struct task_struct* kthread = (struct task_struct*)kmalloc(sizeof(struct task_struct));

    // initialize kernel stack
    kthread->stack = alloc_pages(1);

    // initialize thread_info
    kthread->thread_info.pid = get_pid_counter();
    kthread->thread_info.state = TASK_RUNNING;

    // initialize thread context
    kthread->ctx.lr = (uint64_t)kthread_trampoline;
    kthread->ctx.x19 = (uint64_t)func;
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
    list_add_tail(&kthread->list, &task_list);

    // initialzie singal
    sigpending_init(&kthread->sigpending);
    default_sighand_init(&kthread->sighandler);
    
    LOG("kthread end");
    daif = local_irq_disable_save();
    add_task_to_rq(kthread);
    local_irq_restore(daif);
    return 0;
}

void kthread_start(kthread_func func){
    func();
    kthread_exit();
}

void kthread_exit(){
    volatile uint64_t daif;
    LOG("kthread_exit start");
    struct task_struct* cur;
    cur = get_current();
    cur->thread_info.state = TASK_DEAD;

    daif = local_irq_disable_save();
    list_add_tail(&cur->zombie, &zombies);
    local_irq_restore(daif);
    LOG("kthread_exit end");
    preempt_schedule();
}

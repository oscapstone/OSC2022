#include "sched.h"
#include "exception.h"
#include "malloc.h"

list_head_t *run_queue;
list_head_t *wait_queue;
list_head_t *zombie_queue;

thread_t threads[PIDMAX + 1];

void init_thread_sched()
{
    lock();
    run_queue = kmalloc(sizeof(list_head_t));
    wait_queue = kmalloc(sizeof(list_head_t));
    zombie_queue = kmalloc(sizeof(list_head_t));
    INIT_LIST_HEAD(run_queue);
    INIT_LIST_HEAD(wait_queue);
    INIT_LIST_HEAD(zombie_queue);

    //init pids
    for (int i = 0; i <= PIDMAX; i++)
    {
        threads[i].isused = 0;
        threads[i].pid = i;
        threads[i].iszombie = 0;
    }

    asm volatile("msr tpidr_el1, %0" ::"r"(kmalloc(sizeof(thread_t)))); /// malloc a space for current kernel thread to prevent crash

    thread_t* idlethread = thread_create(idle);
    curr_thread = idlethread;
    unlock();
}

void idle(){
    while(1)
    {
        kill_zombies();   //reclaim threads marked as DEAD
        schedule();       //switch to next thread in run queue
    }
}

void schedule(){
    lock();

    curr_thread = (thread_t*)curr_thread->listhead.next;

    // ignore run_queue head
    if(list_is_head(&curr_thread->listhead,run_queue))
    {
        curr_thread = (thread_t *)curr_thread->listhead.next;
    }

    switch_to(get_current(), &curr_thread->context);
    unlock();
}

void kill_zombies(){
    lock();
    while(!list_empty(zombie_queue))
    {
        list_head_t *curr = zombie_queue->next;
        list_del_entry(curr);
        kfree(((thread_t*)curr)->stack_alloced_ptr); // free stack
        kfree(((thread_t *)curr)->kernel_stack_alloced_ptr); // free stack
        kfree(((thread_t *)curr)->data); // free data
        ((thread_t *)curr) -> iszombie = 0;
        ((thread_t *)curr)-> isused = 0;
    }
    unlock();
}

int exec_thread(char *data, unsigned int filesize)
{
    thread_t *t = thread_create(data);
    t->data = kmalloc(filesize);
    t->datasize = filesize;
    t->context.lr = (unsigned long)t->data;
    //copy file into data
    for (int i = 0; i < filesize;i++)
    {
        t->data[i] = data[i];
    }

    // eret to exception level 0
    asm("msr tpidr_el1, %0\n\t"
        "msr elr_el1, %1\n\t"
        "msr spsr_el1, xzr\n\t" // enable interrupt in EL0. You can do it by setting spsr_el1 to 0 before returning to EL0.
        "msr sp_el0, %2\n\t"
        "mov sp, %3\n\t"
        "eret\n\t" ::"r"(&t->context),"r"(t->context.lr), "r"(t->context.sp), "r"(t->kernel_stack_alloced_ptr + KSTACK_SIZE));

    return 0;
}

thread_t *thread_create(void *start)
{
    lock();

    thread_t *r;
    for (int i = 0; i <= PIDMAX; i++)
    {
        if (!threads[i].isused)
        {
            r = &threads[i];
            break;
        }
    }
    r->iszombie = 0;
    r->isused = 1;
    r->context.lr = (unsigned long long)start;
    r->stack_alloced_ptr = kmalloc(USTACK_SIZE);
    r->kernel_stack_alloced_ptr = kmalloc(KSTACK_SIZE);
    r->context.sp = (unsigned long long )r->stack_alloced_ptr + USTACK_SIZE;
    r->context.fp = r->context.sp;

    list_add(&r->listhead, run_queue);
    unlock();
    return r;
}

void thread_exit(){
    lock();
    list_del_entry(&curr_thread->listhead);
    curr_thread->iszombie = 1;
    list_add(&curr_thread->listhead, zombie_queue);
    schedule();
    unlock();
}
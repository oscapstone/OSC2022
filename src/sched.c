
#include "mini_uart.h"
#include "allocator.h"
#include "sched.h"
#include "exception.h"
#include "cpio.h"
#define TASK_POOL_SIZE 20

extern void switch_to(Thread_struct* prev, Thread_struct* next);
extern Thread_struct* get_current();

Thread_struct* thread_pool;
Thread_struct* run_queue;
Thread_struct* wait_queue;

void thread_create(void (*f)()){
    int i;
    for (i = 0; i < TASK_POOL_SIZE; i++)
    {
        if(thread_pool[i].state == EMPTY){
            break;
        }
    }
    thread_pool[i].state = RUNNING;
    thread_pool[i].user_stack = my_malloc(THREAD_STACK_SIZE);
    thread_pool[i].kernel_stack = my_malloc(THREAD_STACK_SIZE);
    thread_pool[i].cpu_context.lr = (unsigned long)f;
    thread_pool[i].cpu_context.fp = (unsigned long)(thread_pool[i].kernel_stack + THREAD_STACK_SIZE);
    thread_pool[i].cpu_context.sp = (unsigned long)(thread_pool[i].kernel_stack + THREAD_STACK_SIZE);
    writes_uart("Create Thread ");
    write_int_uart(i,TRUE);
    push_thread(&thread_pool[i]);
}
void schedule()
{
    // disable_interrupt();
    if(run_queue != nullptr)
    {
        Thread_struct* next = pop_thread(run_queue);
        // context_switch(next);
        Thread_struct* cur = get_current();
        if (cur->state == RUNNING) 
        {
            push_thread(cur);
        }
        writes_uart("From ");
        write_int_uart(cur->id,FALSE);
        writes_uart(" schedule to ");
        write_int_uart(next->id,TRUE);
        iter_runqueue();
        switch_to(cur,next);
    }
    // enable_interrupt();
}
void context_switch(Thread_struct* next) {

    // Thread_struct *current = get_current();
    // asm volatile("msr sp_el0, %0" : : "r"(&ustack_pool[current->id][USTACK_TOP_IDX]));
    // asm volatile("msr elr_el1, %0": : "r"(func));
    // asm volatile("msr spsr_el1, %0" : : "r"(SPSR_EL1_VALUE));
    // asm volatile("eret");
}
void init_sched()
{
    run_queue = nullptr;
    thread_pool = my_malloc(TASK_POOL_SIZE * sizeof(Thread_struct));
    for (int i = 0; i < TASK_POOL_SIZE; i++)
    {
        thread_pool[i].state = EMPTY;
        thread_pool[i].id = i;
        thread_pool[i].cpu_context.sp = (unsigned long)nullptr;
    }
    thread_pool[0].state = RUNNING; // idle task;
    asm volatile(
        "msr tpidr_el1, %0"
        :: "r" (&thread_pool[0])
    );
}
void idle()
{
    // iter_runqueue();
    while(TRUE)
    {
        kill_zombie();
        schedule();
    }
}
void kill_zombie()
{

}
void thread_exit()
{
    get_current()->state = ZOMBIE;
    schedule();
}
void iter_runqueue()
{
    Thread_struct* node = run_queue;
    while(node != nullptr){
        write_int_uart(node->id,FALSE);
        writes_uart("->");
        node = node->next;
    }
    writes_uart("\r\n");
}
void push_thread(Thread_struct* t)
{
    if(run_queue == nullptr){
        // run_queue = my_malloc(sizeof(Thread_struct));
        run_queue = t;
        run_queue->next = nullptr;
        run_queue->prev = nullptr;
        
    }
    else{
        Thread_struct* node;
        node = run_queue;
        while(node->next != nullptr){
            node = node->next;
        }
        node->next = t;
        t->prev = node;
        t->next = nullptr;
        // writes_uart("Pushed in to queue: ");
        // write_int_uart(t->id,TRUE);
    }
}
Thread_struct* pop_thread()
{
    Thread_struct* q = run_queue;
    if(run_queue!=nullptr){
        if(q->next){
            q->next->prev = nullptr;
        }
        run_queue = run_queue->next;
    }
    return q;
}
void thread_exec()
{
    char *file_start = nullptr;
    unsigned long filesize;
    cpio_get_addr(file_start,&filesize);
    char *new_start = my_malloc(filesize);
    memcpy(new_start,file_start,filesize);
    asm volatile(
        "mov x0, 0\n\t" // 
        "msr spsr_el1, x0\n\t"
        "msr elr_el1, %0\n\t"
        "msr sp_el0,%1\n\t"
        "eret\n\t" // 
        ::"r" (new_start),
        "r" (get_current()->user_stack)
        : "x0"
    );
}
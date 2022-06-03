
#include "mini_uart.h"
#include "allocator.h"
#include "sched.h"
#include "exception.h"
#include "cpio.h"
#include "timer.h"
#include "syscall.h"
#include "signal.h"
#include "vfs.h"
#include "tmpfs.h"
#define TASK_POOL_SIZE 20

extern void switch_to(Thread_struct* prev, Thread_struct* next);

Thread_struct* thread_pool;
Thread_struct* run_queue;
Thread_struct* wait_queue;

int thread_create(void (*f)()){
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
    for(int j=0;j<20;j++)
    {
        thread_pool[i].signal_handler[j] = signal_default_handler;
        thread_pool[i].signal_count[j] = 0;
    }
    thread_pool[i].signal_ustack = nullptr;
    thread_pool[i].run_handler = nullptr;

    for(int j=0;j<20;j++){
        thread_pool[i].fd_table[j]=nullptr;
    }

    writes_uart("Create Thread ");
    write_int_uart(i,TRUE);
    push_thread(&thread_pool[i]);
    return i;
}
void schedule()
{
    disable_interrupt();
    if(run_queue != nullptr)
    {
        Thread_struct* next = pop_thread(run_queue);
        // context_switch(next);
        Thread_struct* cur = get_current();
        if (cur->state == RUNNING && cur->id !=0) 
        {
            push_thread(cur);
        }
        // busy_wait_writes("From ",FALSE);
        // busy_wait_writeint(cur->id,FALSE);
        // busy_wait_writes(" schedule to ",FALSE);
        // busy_wait_writeint(next->id,TRUE);
        // iter_runqueue();
        enable_interrupt();
        // enable_timer_interrupt();
        switch_to(cur,next);
    }
    enable_interrupt();
    return;
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
        busy_wait_writeint(node->id,FALSE);
        busy_wait_writes("->",FALSE);
        node = node->next;
    }
    busy_wait_writes("\r\n",FALSE);
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
    rootfs_init("tmpfs");
    char **file_start = my_malloc(sizeof(char*));
    unsigned long *filesize = my_malloc(sizeof(unsigned long));
    // cpio_get_addr(file_start,&filesize);
    cpio_get_addr(file_start,filesize);
    char *new_start;
    
    new_start = my_malloc(*filesize);
    memcpy(new_start,*file_start,*filesize);
    Thread_struct* cur_thread = get_current();
    asm volatile(
        "mov x0, 0\n\t" // 
        "msr spsr_el1, x0\n\t"
        "msr elr_el1, %0\n\t"
        "msr sp_el0,%1\n\t"
        "mov sp, %2\n\t"
        "msr tpidr_el1, %3\n\t"
        ::"r" (new_start),
        "r" ((char*)(cur_thread->user_stack+THREAD_STACK_SIZE)),
        "r" ((char*)(cur_thread->kernel_stack+THREAD_STACK_SIZE)),
        "r" ((char*)cur_thread)
        : "x0"
    );
    
    asm volatile(
        "eret\n\t" // 
    );
}
Thread_struct* get_thread(int id){
    return &(thread_pool[id]);
}
void kill_thread(int id)
{
    thread_pool[id].state = ZOMBIE;
}
void raise_signal(int pid,int signal)
{
    thread_pool[pid].signal_count[signal]++;
}
#include <sched.h>
#include <process.h>
#include <stdint.h>
#include <kmalloc.h>
#include <interrupt.h>
#include <queue.h>
#include <uart.h>
#include <timer.h>
#include <lock.h>
#include <signal.h>
#include <error.h>

extern void context_switch_to(void *, void *, uint64_t);

Thread *all_threads;
uint64_t thread_num;
Queue *run_queue;
//Lock *run_queue_lock;
Lock *thread_num_lock;

#define thread_flag_running 0b1
#define thread_flag_wait 0b10
#define thread_flag_zombie 0b100
#define thread_readflag(t,f) ( ((Thread*)t)->status & f )
#define thread_setflag(t,f) ( ((Thread*)t)->status |= f )
#define thread_unsetflag(t,f) ( ((Thread*)t)->status &= ~f )

void sched_init()
{
    run_queue = queue_new();
    //run_queue_lock = lock_new();
    thread_num_lock = lock_new();
    all_threads = 0;
    thread_num = 0;
}

void run_queue_push(Thread *thread)
{
    //if(!run_queue) run_queue = queue_new();
    //thread_setflag(thread, thread_flag_running);
    thread->status = ThreadStatus_running;
    //lock_get(run_queue_lock);
    queue_push(run_queue, thread);
    //kmsg("push thread into runqueue: 0x%x", thread);
    //lock_release(run_queue_lock);
}

Thread* get_thread(uint64_t tid)
{
    Thread *cur_t = all_threads;
    while(cur_t){
        if(cur_t->thread_id==tid){
            return cur_t;
        }
        cur_t = cur_t->fd;
    }
    return 0;
}

void thread_wrapper()
{
    uart_puts("thread_wrapper()");
    uint64_t tid = thread_get_current();
    uart_print("thread_wrapper(): tid=0x");
    uart_putshex(tid);
    Thread *thread = get_thread(tid);
    uart_print("thread_wrapper(): thread=0x");
    uart_putshex(thread);
    uart_print("thread_wrapper(): thread->func=0x");
    uart_putshex(thread->func);
    uart_print("thread_wrapper(): thread->arg=0x");
    uart_putshex(thread->arg);
    interrupt_enable();
    thread->func(thread->arg);
    interrupt_disable();
    //thread_unsetflag(thread, thread_flag_running);
    //thread_setflag(thread, thread_flag_zombie);
    thread->status = ThreadStatus_zombie;
    interrupt_enable();
    schedule();
}

void thread_run(Thread *thread)
{
    //interrupt_disable();
    run_queue_push(thread);
    //interrupt_enable();
}

Thread* create_thread(void (*func)(void *), void* arg)
{
    // interrupt_disable();
    Thread *thread = (Thread*)kmalloc(sizeof(Thread));
    //interrupt_disable();
    lock_get(thread_num_lock);
    thread->thread_id = ++thread_num;
    lock_release(thread_num_lock);
    //interrupt_enable();
    thread->status = ThreadStatus_init;
    thread->arg = arg;
    for(int i=0;i<(sizeof(CPUState) / sizeof(uint64_t));i++) ((uint64_t*)(&(thread->saved_reg)))[i] = 0;
    thread->stack_pointer = buddy_alloc(2); // 2 pages = 8 * 1024
    thread->func = func;
    thread->saved_reg.sp = (uint64_t)((void **)thread->stack_pointer + 1022);
    thread->saved_reg.lr = (uint64_t)thread_wrapper;
    thread->saved_reg.fp = thread->saved_reg.sp;
    thread->fd = 0;
    thread->process = 0;
    if(all_threads){
        thread->fd = all_threads;
    }
    all_threads = thread;
    
    return thread;
    // interrupt_enable();
}

void schedule_init()
{
    //CPUState saved_reg;
    CPUState *saved_reg = (CPUState *)kmalloc(sizeof(CPUState));
    uart_puts("schedule_init()");
    interrupt_disable();
    //Thread *cur_thread = get_thread(thread_get_current());
    Thread *new_thread = queue_pop(run_queue);
    uart_print("schedule_init(): new_thread: 0x");
    uart_putshex(new_thread);
    uart_print("schedule_init(): new_thread->thread_id: 0x");
    uart_putshex(new_thread->thread_id);
    uart_print("schedule_init(): saved_reg: 0x");
    uart_putshex(saved_reg);
    uart_print("schedule_init(): &(new_thread->saved_reg): 0x");
    uart_putshex(&(new_thread->saved_reg));
    uart_print("schedule_init(): new_thread->saved_reg.sp: 0x");
    uart_putshex(new_thread->saved_reg.sp);
    uart_print("schedule_init(): new_thread->saved_reg.fp: 0x");
    uart_putshex(new_thread->saved_reg.fp);
    uart_print("schedule_init(): new_thread->saved_reg.lr: 0x");
    uart_putshex(new_thread->saved_reg.lr);
    //queue_pop(run_queue);
    context_switch_to(saved_reg, &(new_thread->saved_reg), new_thread->thread_id);
    uart_puts("schedule_init(): context switch error");
    interrupt_enable();
}

void schedule()
{
    interrupt_disable();
    //uart_puts("schedule()");
    Thread *cur_thread = get_thread(thread_get_current());
    Thread *new_thread;
    do{
        new_thread = queue_pop(run_queue);
        //kmsg("find thread to switch to: thread=0x%x", new_thread);
        //queue_pop(run_queue);
    } while(new_thread && new_thread->status == ThreadStatus_zombie && new_thread != 0);
    if(new_thread){
        //uart_print("Switch to thread 0x");
        //uart_putshex(new_thread->thread_id);
        //kmsg("Prev Thread: 0x%x (tid=0x%x, status=%d)", cur_thread, cur_thread->thread_id, cur_thread->status);
        if(cur_thread->status == ThreadStatus_running){
            run_queue_push(cur_thread);
        }
        signal_check();
        context_switch_to(&(cur_thread->saved_reg), &(new_thread->saved_reg), new_thread->thread_id);
        signal_check();
    }
    interrupt_enable();
}

void _kill_zombie(Thread *thread, Thread *pre)
{
    //uart_print("_kill_zombie(): Free stack 0x");
    //uart_putshex((uint64_t)thread->stack_pointer);
    //uart_print("_kill_zombie(): killing zombie thread 0x");
    //uart_putshex((uint64_t)thread->thread_id);
    kmsg("killing zombie thread %x", thread->thread_id);
    if(thread->process && thread->process->signal_tid != thread->thread_id){
        if(thread->process->sigstack)buddy_free(thread->process->sigstack);
        if(thread->process->signal_tid){
            Thread *sigthread = get_thread(thread->process->signal_tid);
            sigthread->status = ThreadStatus_zombie;
        }
        process_free(thread->process);
        thread->process = 0;
    }
    buddy_free(thread->stack_pointer);
    if(pre) pre->fd = thread->fd;
    kfree(thread);
    uart_puts("_kill_zombie(): killed.");
}

void kill_zombies()
{
    interrupt_disable();
    Thread *cur_t = all_threads;
    Thread *pre_t = 0;
    while(cur_t){
        if(cur_t->status == ThreadStatus_zombie){
            Thread *nxt_t = cur_t->fd;
            if(pre_t==0) all_threads = cur_t->fd;
            _kill_zombie(cur_t, pre_t);
            cur_t = nxt_t;
        }else{
            pre_t = cur_t;
            cur_t = cur_t->fd;
        }
    }
    interrupt_enable();
}

void idle_thread()
{
    while(1){
        //kmsg("Check zombies.");
        kill_zombies();
        schedule();
    }
}

//void reboot_now();

void thread_start(void (*func)(void *))
{
    uart_puts("thread_start(): create first thread.");
    Thread* thread = create_thread(func, 0);
    //Thread* thread = create_thread(reboot_now, 0);
    uart_puts("thread_start(): create idle thread.");
    Thread* idle_t = create_thread(idle_thread, 0);
    thread_run(thread);
    thread_run(idle_t);
    uart_puts("Thread Start!!");
    schedule_init();
}

void wait(Queue *waitqueue)
{
    Thread* thread = get_thread(thread_get_current());
    //thread_unsetflag(thread, thread_flag_running);
    //thread_setflag(thread, thread_flag_wait);
    signal_check();
    thread->status = ThreadStatus_wait;
    //interrupt_disable();
    queue_push(waitqueue, thread);
    //interrupt_enable();
    schedule();
}

void waitlock(LockQueue *waitqueue)
{
    Thread* thread = get_thread(thread_get_current());
    thread->status = ThreadStatus_wait;
    lockqueue_push(waitqueue, thread);
    schedule();
}

void wait_thread(Thread *thread)
{
    thread->status = ThreadStatus_wait;
    schedule();
}

void wakeup(Queue *waitqueue)
{
    //interrupt_disable();
    Thread* thread = queue_pop(waitqueue);
    //queue_pop(waitqueue);
    if(thread){
        //thread_unsetflag(thread, thread_flag_wait);
        thread->status = ThreadStatus_running;
        run_queue_push(thread);
    }
    //interrupt_enable();
}

void wakeuplock(LockQueue *waitqueue)
{
    Thread* thread = lockqueue_pop(waitqueue);
    if(thread){
        thread->status = ThreadStatus_running;
        run_queue_push(thread);
    }
}


void wakeup_thread(Thread *thread)
{
    //interrupt_disable();
    thread->status = ThreadStatus_running;
    run_queue_push(thread);
    //interrupt_enable();
}

void sched_preempt()
{
    //uart_puts("Preempt");
    add_timer(31, sched_preempt, 0);
    schedule();
}
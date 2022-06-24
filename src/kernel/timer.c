#include <timer.h>
#include <stdint.h>
#include <mmio.h>
#include <uart.h>
#include <kmalloc.h>
#include <interrupt.h>
#include <sched.h>

TIMER* timer_queue;

void coretimer_el0_enable()
{
    uint64_t tmp;
    asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));

    asm("msr cntp_ctl_el0, %0"::"r"((uint64_t)1));
    //mmio_set(CORE0_TIMER_IRQ_CTRL, 2); //nCNTPNSIRQ IRQ Enable CNT Physical Not Secure Timer IRQ(?
    //asm("mrs x0, cntfrq_el0");
    //asm("msr cntp_tval_el0, x0");
    //coretimer_el0_set(1000);
}

inline static void _coretimer_el0_set(uint32_t t)
{
    asm("msr cntp_tval_el0, %0"::"r"(t));
}

void coretimer_el0_set(uint32_t t)
{
    uint32_t cntfrq_el0;
    asm("mrs %0, cntfrq_el0":"=r"(cntfrq_el0));
    uint32_t val = t * (cntfrq_el0 / 1000);
    _coretimer_el0_set(val);
}

void timer_queue_push(TIMER* new_timer)
{
    interrupt_disable();
    if(!timer_queue){
        timer_queue = new_timer;
        goto ret;
    }
    TIMER* head = timer_queue;
    TIMER* prev = 0;
    if(head->time > new_timer->time){
        new_timer->next = head;
        timer_queue = new_timer;
        goto ret;
    }
    while(head->next){
        prev = head;
        head = head->next;
        if(head->time > new_timer->time){
            new_timer->next = head;
            prev->next = new_timer;
        goto ret;
        }
    }
    head->next = new_timer;
ret:
    interrupt_enable();
}

void timer_queue_pop()
{
    if(!timer_queue) return ;
    interrupt_disable();
    TIMER* timer = timer_queue;
    timer_queue = timer_queue->next;
    interrupt_enable();
    kfree(timer);
    //return timer;
}

TIMER* timer_queue_top()
{
    if(!timer_queue) return 0;
    return timer_queue;
}

void timer_sched()
{
    TIMER* timer = timer_queue_top();
    //if(!timer) return ;
    if(!timer){
        uart_puts("timer_sched(): no timer to sched");
        add_timer(31, sched_preempt, 0);
        return ;
    }
    uint64_t cntpct_el0;
    asm("mrs %0, cntpct_el0":"=r"(cntpct_el0));
    _coretimer_el0_set(timer->time-cntpct_el0);
    mmio_set(CORE0_TIMER_IRQ_CTRL, 2);
}

void _add_timer(uint64_t time, void (*func)(void *), void *arg)
{
    TIMER* new_timer = (TIMER*)kmalloc(sizeof(TIMER));
    new_timer->time = time;
    new_timer->func = func;
    new_timer->arg = arg;
    new_timer->next = 0;

    timer_queue_push(new_timer);
    timer_sched();
}

void add_timer(uint64_t time_wait, void (*func)(void *), void *arg) //ms
{
    uint32_t cntfrq_el0;
    uint64_t cntpct_el0;
    asm("mrs %0, cntfrq_el0":"=r"(cntfrq_el0));
    asm("mrs %0, cntpct_el0":"=r"(cntpct_el0));

    // _add_timer(cntpct_el0+time_wait*cntfrq_el0, func, arg);
    _add_timer(cntpct_el0+((time_wait*cntfrq_el0)/1000), func, arg);
}

void coretimer_el0_handler()
{
    mmio_set(CORE0_TIMER_IRQ_CTRL, 0);
    uint64_t cntpct_el0;
    asm("mrs %0, cntpct_el0":"=r"(cntpct_el0));

    TIMER* timer;
    while(1){
        interrupt_disable();
        timer = timer_queue_top();
        if(!timer){
            interrupt_enable();
            break;
        }
        if(timer->time > cntpct_el0) break;
        timer_queue_pop();
        interrupt_enable();
        timer->func(timer->arg);
    }
    timer_sched();
}
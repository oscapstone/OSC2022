#include <task.h>
#include <uart.h>
#include <string.h>
#include <malloc.h>
#include <irq.h>

Task *task_head = NULL;
void add_task(Handler handler, unsigned int priority){
    // unsigned long long system_timer = 0;
    // unsigned long long frq = 0;

    // asm volatile(
    //     "mrs %0, cntpct_el0\n\t"
    //     "mrs %1, cntfrq_el0\n\t" 
    //     :"=r"(system_timer), "=r"(frq)
    // );

    Task *task = (Task*)simple_malloc(sizeof(Task));
    task->handler = handler;
    task->priority = priority;
    task->next = NULL;
    task->prev = NULL;

    if(task_head == NULL){
        task_head = task;
    }
    else{
        Task *tmp = task_head;
        while(tmp->next != NULL && task->priority > tmp->priority){
            tmp = tmp->next;
        }

        if(task->priority < tmp->priority){
            if(tmp == task_head){
                task_head = task;
                task_head->next = tmp;
                tmp->prev = task_head;
            }
            else{
                task->next = tmp;
                task->prev = tmp->prev;
                tmp->prev->next = task;
                tmp->prev = task;
            }
        }
        else{
            tmp->next = task;
            task->prev = tmp;
        }

    }
}


void do_task(){
    // for(Task *tmp = task_head; tmp != NULL; tmp = tmp->next){
    //     uart_tputs("->");
    //     if(tmp->priority == 2) uart_tputs("time");
    //     else uart_tputs("uart");
    // }
    // uart_tputs("\n");

    while(task_head != NULL){
        if(task_head->next != NULL){
            Handler handler = task_head->handler;
            task_head = task_head->next;
            task_head->prev = NULL;
            enable_irq();
            handler();
        } 
        else{
            Handler handler = task_head->handler;
            task_head = NULL;
            enable_irq();
            handler();
        }
    }
    
    // if(head == NULL) set_long_timer_irq();
}
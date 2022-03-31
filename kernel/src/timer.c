#include <timer.h>
#include <uart.h>
#include <string.h>
#include <malloc.h>
#include <irq.h>

Timer *head = NULL;
void add_timer(TimerTask task, unsigned long long expired_time, void *args){
    unsigned int change_time = 0;
    unsigned long long system_timer = 0;
    unsigned long long frq = 0;

    asm volatile(
        "mrs %0, cntpct_el0\n\t"
        "mrs %1, cntfrq_el0\n\t" 
        :"=r"(system_timer), "=r"(frq)
    );

    Timer *timer = (Timer*)simple_malloc(sizeof(Timer));
    timer->expired_time = system_timer + expired_time * frq;
    timer->task = task;
    timer->args = args;
    timer->next = NULL;
    timer->prev = NULL;

    if(head == NULL){
        head = timer;
        change_time = 1;
    }
    else{
        Timer *tmp = head;
        while(tmp->next != NULL && timer->expired_time > tmp->expired_time){
            tmp = tmp->next;
        }

        if(timer->expired_time < tmp->expired_time){
            if(tmp == head){
                head = timer;
                head->next = tmp;
                tmp->prev = head;
                change_time = 1;
            }
            else{
                timer->next = tmp;
                timer->prev = tmp->prev;
                tmp->prev->next = timer;
                tmp->prev = timer;
            }
        }
        else{
            tmp->next = timer;
            timer->prev = tmp;
        }

    }

    if(change_time) reset_timer_irq(head->expired_time);

}


void timer_interrupt_handler(){

    while(head != NULL){
        if(head->next != NULL){
            head->task(head->args);
            head = head->next;
            head->prev = NULL;
            unsigned long long system_timer = 0;
            asm volatile("mrs %0, cntpct_el0\n\t" :"=r"(system_timer));
            if(head->expired_time > system_timer){
                reset_timer_irq(head->expired_time);
                break;
            }
        } 
        else{
            head->task(head->args);
            head = NULL;
        }
    }
    if(head == NULL) set_long_timer_irq();
}

void timeout_print(void *args){
    uart_puts((char*)args);
}
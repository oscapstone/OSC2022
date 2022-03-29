#include <timer.h>
#include <uart.h>
#include <string.h>
#include <malloc.h>

Timer *head = NULL;
void add_timer(TimerTask task, unsigned long long expired_time, void *args){
    unsigned int change_time = 0;
    unsigned long long system_timer = 0;
    asm volatile("mrs %0, cntpct_el0\n\t" ::"r"(system_timer));
    Timer *timer = (Timer*)simple_malloc(sizeof(Timer));
    timer->task = task;
    timer->expired_time = system_timer + expired_time;
    timer->next = NULL;
    timer->prev = NULL;

    if(head == NULL){
        head = timer;
        change_time = 1;
    }

    else{
        Timer *tmp = head;
        while(tmp->next != NULL){
            if(timer->expired_time < tmp->expired_time){
                if(tmp == head){
                    timer->next = tmp;
                    timer->prev = NULL;
                    tmp->next = NULL;
                    tmp->prev = timer;
                    head = timer;
                    change_time = 1;
                }
                else{
                    timer->next = tmp;
                    timer->prev = tmp->prev;
                    tmp->prev->next = timer;
                    tmp->prev = timer;
                }
            }
            tmp = tmp->next;
        }
    }

    if(change_time){
        asm volatile(
            "msr cntp_tval_el0, %0\n\t" 
            ::"r"(expired_time)
        );
    }

}

void timeout_print(void *args){
    uart_puts(args);
}
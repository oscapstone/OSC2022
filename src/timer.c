#include "timer.h"
#include "mini_uart.h"
#include "stdlib.h"
#include "shell.h"
#include "string.h"
#define CORE0_TIMER_IRQ_CTRL ((volatile unsigned int*)(0x40000040))
void init_timer(){
    // head->next = nullptr;
    // head->value = null;
    
    head = nullptr;
    // writes_uart("Core timer: ");
    //writehex_uart(*CORE0_TIMER_IRQ_CTRL,1);
}
void get_current_time(unsigned long long *time_count,unsigned long long *time_freq){
    unsigned long long _time_count=0;
    unsigned long long _time_freq=0;
    asm volatile(
        "mrs %0,cntpct_el0\n\t"
        "mrs %1,cntfrq_el0\n\t"
        :"=r" (_time_count),
         "=r" (_time_freq)
        );
    *time_count = _time_count;
    *time_freq = _time_freq;
}
void set_expired_time(int after){
    unsigned long long time_count=0;
    unsigned long long time_freq=0;
    get_current_time(&time_count,&time_freq);
    // write_int_uart((int)(time_count/time_freq),1);
    unsigned long long time_expired = time_freq*after;
    // write_int_uart(time_expired,1);
    asm volatile(
        "msr cntp_tval_el0, %0\n\t"
        ::"r" (time_expired)
    );
    return;
}

void add_timer(void (*callback)(char* s),char *message,int after){
    //busy_wait_writes("Enter add timer",TRUE);
    timer* node ;
    unsigned long long time_count=0;
    unsigned long long time_freq=0;
    // char* tmp_message = simple_malloc(strlen(message));
    // for (int i = 0; i < strlen(message); i++)
    // {
    //     tmp_message[i] = message[i];
    // }
    
    if(head==nullptr){
        writes_nl_uart("Add new timer to null queue");
        node = (timer*)simple_malloc(sizeof(timer));
        node->next=nullptr;
        get_current_time(&time_count,&time_freq);
        node->value=(time_count/time_freq) + after;
        node->message = message;
        node->callback = callback;
        head = node;
        set_expired_time(after);
        enable_timer_interrupt();
    }else{
        // busy_wait_writes("ADDing new timer busy2",TRUE);
        writes_nl_uart("Add new timer to non-null queue");
        timer* newNode = (timer*)simple_malloc(sizeof(timer));
        get_current_time(&time_count,&time_freq);
        newNode->value=(time_count/time_freq) + after;
        newNode->message=message;
        newNode->callback = callback;
        if((time_count/time_freq + after) < head->value){
            newNode->next = head;
            head = newNode;
            set_expired_time(after);
        }
        else{
            timer *node = head;
            get_current_time(&time_count,&time_freq);
            while(node->next!=null){
                if(node->next->value>=(time_count/time_freq + after)){
                    break;
                }
                node = node->next;
            }
            newNode->next = node->next;
            node->next = newNode;
        }
    }
    //callback(message);
    itr_timer_queue();
    return;
}

void itr_timer_queue(){
    timer *node = head;
    while(node!=nullptr){
        writes_uart(node->message);
        writes_uart(" ");
        write_int_uart(node->value,TRUE);
        node = node->next;
    }
    return;
}

bool timer_is_empty(){
    if(head==null){
        return TRUE;
    }
    else{
        return FALSE;
    }
}

void disable_timer_interrupt(){
    // head = nullptr;
    *CORE0_TIMER_IRQ_CTRL = 0;
    // asm volatile(
    //     "mrs x0, cntfrq_el0\n\t"
    //     "mov x1,0x00000fff\n\t"
    //     "mul x0,x0,x1\n\t"
    //     "msr cntp_tval_el0, x0\n\t"
    //     //"mov x0, 0\n\t"
    //     //"msr cntp_ctl_el0, x0\n\t"
    //     :::"x0","x1"
    // );

    //*CORE0_TIMER_IRQ_CTRL &= ~(1 << 2);

}
void enable_timer_interrupt(){
    *CORE0_TIMER_IRQ_CTRL = 2;
    // asm volatile(
    //     "mov x0,1\n\t"
    //     "msr cntp_ctl_el0, x0\n\t"
    //     ::
    //     : "x0"
    // );
    // asm volatile(
    //     "bl core_timer_enable"
    // );
}
bool is_timerq_empty(){
    if(head==nullptr)
        return TRUE;
    else
        return FALSE;
}

timer* to_next_timer(){
    head = head->next;
    return head;
}
timer* get_head_timer(){
    return head;
}
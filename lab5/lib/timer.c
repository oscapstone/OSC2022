#include "timer.h"
#include "printf.h"
#include "malloc.h"
#include "string.h"
#include "uart.h"
#include "scheduler.h"

static timer_list *timer_queue = NULL;

task_list *task_queue = NULL;

void interrupt_enable(void){
  __asm__ __volatile__(
    "msr DAIFClr, 0xf" // enable interrupt el1 -> el1
  ); 
}

void interrupt_disable(void){
  __asm__ __volatile__(
    "msr DAIFSet, 0xf" // enable interrupt el1 -> el1
  ); 
}

void core_timer_enable(void){
  uint64_t tmp;
  asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
  tmp |= 1;
  asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
  __asm__ __volatile__(
    "mov x1, 1\n\t"
    "msr cntp_ctl_el0, x1\n\t" // enable Counter-timer Physical Timer Control
  );
}

void core_timer_interrupt_enable(void){
  __asm__ __volatile__(
    "mov x2, 2\n\t"
    "ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n\t"
    "str w2, [x1]\n\t" // unmask timer interrupt
  );
}

void core_timer_interrupt_disable(void){
  __asm__ __volatile__(
    "mov x2, 0\n\t"
    "ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n\t"
    "str w2, [x1]\n\t" // unmask timer interrupt
  );
}

uint64_t get_timer_tick(){
  uint64_t cntpct_el0;
  __asm__ __volatile__(
    "mrs %0, cntpct_el0\n\t"
    : "=r"(cntpct_el0)
  ); //tick now
  return cntpct_el0;
}

uint64_t get_timer_freq(){
  uint64_t cntfrq_el0;
  __asm__ __volatile__(
    "mrs %0, cntfrq_el0\n\t"
    : "=r"(cntfrq_el0)
  ); //tick frequency
  return cntfrq_el0;
}

uint64_t clock_time_s(void){
  uint64_t cntpct_el0 = get_timer_tick();
  uint64_t cntfrq_el0 = get_timer_freq();
  return cntpct_el0 / cntfrq_el0;
}

void set_core_timer_interrupt(uint64_t expired_time){
  __asm__ __volatile__(
    // "mrs x1, cntfrq_el0\n\t"       //cntfrq_el0 -> relative time
    // "mul x1, x1, %0\n\t"
    "msr cntp_tval_el0, %0\n\t"    // set expired time
    : "=r"(expired_time)
  );
}

void add_timer(callback_typ callback, char *msg, int time) {
  timer_list *timer = (timer_list*)malloc(sizeof(timer_list));
  timer->expired_time = (unsigned long long)time + clock_time_s();
  timer->call_back = callback;
  for(int i=0; i<=strlen(msg); i++)
    timer->msg[i] = *(msg+i);
  timer->next = NULL;
  if(!timer_queue){               // be the haed
    timer_queue = timer;
    set_core_timer_interrupt(time);
    core_timer_interrupt_enable();
  }
  else if (timer_queue->expired_time > timer->expired_time) { // add to head
    timer->next = timer_queue;
    timer_queue = timer;
    set_core_timer_interrupt(time);
    core_timer_interrupt_enable();
  }
  else{                           // insert
    timer_list *pre = timer_queue, *next = timer_queue->next;
    while (next && pre->expired_time < timer->expired_time) {
      pre = next;
      next = next->next;
    }
    pre->next = timer;
    timer->next = next;
  }
}

void pop_timer(void){
  timer_list *timer = timer_queue;
  timer_queue = timer_queue->next;
  timer->call_back(timer->msg);
  free(timer);
  if (!timer_queue){
    core_timer_interrupt_disable();
  }else{
    set_core_timer_interrupt(timer_queue->expired_time - clock_time_s());
    core_timer_interrupt_enable();
  }
}

void normal_timer(){
  add_timer(normal_timer, "normal_timer", get_timer_freq()>>5);
  interrupt_enable();
  schedule();
}

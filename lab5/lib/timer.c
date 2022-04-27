#include "timer.h"
#include "printf.h"
#include "malloc.h"
#include "string.h"

static timer_list *timer_queue = NULL;

task_list *task_queue = NULL;

void core_timer_enable(void){
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

unsigned long clock_time(void){
  uint64_t cntpct_el0;
  __asm__ __volatile__(
    "mrs %0, cntpct_el0\n\t"
    : "=r"(cntpct_el0)
  ); //tick now

  uint64_t cntfrq_el0;
  __asm__ __volatile__(
    "mrs %0, cntfrq_el0\n\t"
    : "=r"(cntfrq_el0)
  ); //tick frequency
  return cntpct_el0 / cntfrq_el0;
}

void clock_alert(char *str){
  uint64_t cntpct_el0;
  __asm__ __volatile__(
    "mrs %0, cntpct_el0\n\t"
    : "=r"(cntpct_el0)
  ); //tick now

  uint64_t cntfrq_el0;
  __asm__ __volatile__(
    "mrs %0, cntfrq_el0\n\t"
    : "=r"(cntfrq_el0)
  ); //tick frequency
  printf("This is form clock_alert function.\n\r");
  printf("seconds after booting : %d\n\r", cntpct_el0 / cntfrq_el0);
  // set_core_timer_interrupt(2);
  add_timer(clock_alert, "clock", 2);
}

void timeout_print(char *str){
  printf("This is form timeout function.\n\r");
  print_time();
  printf("Message: %s.\n\r", str);
}

void print_time(void){
  uint64_t cntpct_el0;
  __asm__ __volatile__(
    "mrs %0, cntpct_el0\n\t"
    : "=r"(cntpct_el0)
  ); //tick now

  uint64_t cntfrq_el0;
  __asm__ __volatile__(
    "mrs %0, cntfrq_el0\n\t"
    : "=r"(cntfrq_el0)
  ); //tick frequency
  printf("seconds after booting : %d\n\r", cntpct_el0 / cntfrq_el0);
}

void set_core_timer_interrupt(uint64_t expired_time){
  __asm__ __volatile__(
    "mrs x1, cntfrq_el0\n\t" //cntfrq_el0 -> relative time
    "mul x1, x1, %0\n\t"
    "msr cntp_tval_el0, x1\n\t" // set expired time
    : "=r"(expired_time)
  );
}

void add_timer(callback_typ callback, char *msg, int time) {
  timer_list *timer = (timer_list*)malloc(sizeof(timer_list));
  timer->expired_time = (uint64_t)time + clock_time();
  timer->call_back = callback;
  for(int i=0; i<=strlen(msg); i++)
    timer->msg[i] = *(msg+i);
  // timer->msg[strlen(msg)] = 0;
  timer->next = NULL;
  if(!timer_queue){ // be the haed
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
  else{ // insert
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
  // add_task(timer->call_back, timer->msg, 1);
  if (!timer_queue)
    core_timer_interrupt_disable();
  else{
    set_core_timer_interrupt(timer_queue->expired_time - clock_time());
    core_timer_interrupt_enable();
  }
}

void add_task(callback_typ callback, char *msg, int piority){
  task_list *task = (task_list*)malloc(sizeof(task_list));
  task->task_call_back = callback;
  task->arg = msg;
  task->piority = piority;
  task->next = NULL;
  if(!task_queue)
    task_queue = task;
  else{
    task_list *pre = task_queue, *next = task_queue->next;
    while (next) {
      pre = next;
      next = next->next;
    }
    pre->next = task;
  }
}

void pop_task(void){
  task_list *task = task_queue;
  if(task_queue){
    task_queue = task_queue->next;
    task->task_call_back(task->arg);
  }
}

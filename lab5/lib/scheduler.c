#include "printf.h"
#include "scheduler.h"
#include "malloc.h"
#include "shell.h"
#include "timer.h"
#include "delay.h"
#include "system.h"
#include "uart.h"
#include "mailbox.h"

task *run_queue = NULL;
task *zombies_queue = NULL;

static int pid = 1;
static void enqueue(task **queue, task *new_task);
static void *dequeue(task **queue);

void thread_init(void){
  task *new_task = malloc(sizeof(task));
  new_task->pid = pid++;
  new_task->next = NULL;
  new_task->state = RUNNING;
  enqueue(&run_queue, new_task);
  new_task = malloc(sizeof(task));
  new_task->pid = pid++;
  new_task->next = NULL;
  new_task->state = RUNNING;
  new_task->lr = (uint64_t)shell;
  char *addr = malloc(THREAD_SP_SIZE);
  new_task->fp = (uint64_t)addr;
  new_task->sp = (uint64_t)addr;
  enqueue(&run_queue, new_task);
}

void foo(){
  for(int i = 0; i < 3; ++i) {
    printf("sys_getpid: %d, %d\n\r", sys_getpid(), i);
    if(sys_getpid() == 5 && i == 1){
      sys_fork();
    }
    delay_tick(10000000);
  }
  sys_exit();
}

void idle_thread(void){
  for(int i = 0; i < 3; ++i) { 
    task_create(foo, USER);
  }
  write_current((uint64_t)dequeue(&run_queue));
  add_timer(normal_timer, "normal_timer", get_timer_freq()>>5);
  while (1){
    schedule();
    kill_zombies();
  }
}

void *task_create(thread_func func, enum mode mode){
  task *new_task = malloc(sizeof(task));
  new_task->mode = mode;
  new_task->next = NULL;
  new_task->pid = pid++;
  new_task->state = RUNNING;
  if(mode == USER){
    char *addr = malloc(THREAD_SP_SIZE);
    new_task->user_sp = (uint64_t)addr;
    addr = addr + THREAD_SP_SIZE - 16;
    new_task->lr = (uint64_t)switch_to_user_space;
    new_task->target_func = (uint64_t)func;
  }else{
    new_task->lr = (uint64_t)func;
  }
  char *addr = malloc(THREAD_SP_SIZE);
  new_task->sp_addr = (uint64_t)addr;
  addr = addr + THREAD_SP_SIZE - 16;
  new_task->fp = (uint64_t)addr;
  new_task->sp = (uint64_t)addr;
  enqueue(&run_queue, new_task);
  return new_task;
}

void schedule(){
  task *cur = get_current();
  cur->next = NULL;
  if(cur->state != EXIT)
    enqueue(&run_queue, cur);
  else
    enqueue(&zombies_queue, cur);
  task *next = dequeue(&run_queue);
  switch_to(cur, next);
}

void kill_zombies(){
  task* cur = dequeue(&zombies_queue);
  if(cur != NULL){
    free(cur);
    free((char *)cur->sp_addr);
    if(cur->mode == USER)
      free((char *)cur->user_sp);
  }
}

void enqueue(task **queue, task *new_task){
  task *cur = *queue;
  task *prev = cur;
  new_task->next = NULL;
  if(*queue  == NULL){
    *queue = new_task;
  }else{
    while (cur){
      prev = cur;
      cur = cur->next;
    }
    prev->next = new_task;
  }
}

void *dequeue(task **queue){
  task *pop_task = *queue;
  *queue = (*queue)->next;
  return pop_task;
}

void kill_thread(int pid){
  task *cur = run_queue;
  task *pre = NULL;
  while (cur){
    if(cur->pid == pid){
      if(pre != NULL)
        pre->next = cur->next;
      else
        run_queue = cur->next;
      enqueue(&zombies_queue, cur);
      break;
    }
    pre = cur;
    cur = cur->next;
  }
  return;
}

void switch_to_user_space() {
  task *cur = get_current();
  asm volatile("mov x0, 0   \n"::);
  asm volatile("msr spsr_el1, x0   \n"::);
  asm volatile("msr elr_el1,  %0   \n"::"r"(cur->target_func));
  asm volatile("msr sp_el0,   %0   \n"::"r"(cur->user_sp - cur->user_sp%16));
  asm volatile("eret  \n"::);
}

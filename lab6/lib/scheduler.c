#include "scheduler.h"
#include "printf.h"
#include "malloc.h"
#include "shell.h"
#include "timer.h"
#include "system.h"

static task *run_queue = NULL;
static task *zombies_queue = NULL;
static int pid = 1;
static void enqueue(task **queue, task *new_task);
static void *dequeue(task **queue);

void thread_init(void){
  task_create(idle_thread, KERNEL);
}

void idle_thread(void){
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
  new_task->handler = NULL;
  new_task->state = RUNNING;
  if(mode == USER){
    char *addr = malloc(THREAD_SP_SIZE);
    new_task->user_sp = (uint64_t)addr;
    addr += THREAD_SP_SIZE - 1;
    addr -= (uint64_t)addr%0x10;
    new_task->lr = (uint64_t)switch_to_user_space;
    new_task->target_func = (uint64_t)func;
  }else{
    new_task->lr = (uint64_t)func;
  }
  char *addr = malloc(THREAD_SP_SIZE);
  new_task->sp_addr = (uint64_t)addr;
  addr += THREAD_SP_SIZE - 1;
  addr -= (uint64_t)addr%0x10;
  new_task->fp = (uint64_t)addr;
  new_task->sp = (uint64_t)addr;
  enqueue(&run_queue, new_task);  
  return new_task;
}

void schedule(){
  task *cur = get_current();
  task *next = dequeue(&run_queue);
  cur->next = NULL;
  if (cur->pid == 1 && !next){
    pid = 2;
    enqueue(&run_queue, cur);
    core_timer_interrupt_disable();
    shell();
  }
  if(cur->state != EXIT)
    enqueue(&run_queue, cur);
  else
    enqueue(&zombies_queue, cur);
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
  if(pop_task){
    *queue = (*queue)->next;
    return pop_task;
  }
  return NULL;
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
  asm volatile("msr elr_el1,  %[output]   \n"::[output]"r"(cur->target_func));
  uint64_t addr = cur->user_sp;
  addr += THREAD_SP_SIZE - 1;
  addr -= addr % 0x100;
  asm volatile("msr sp_el0,   %[output]   \n"::[output]"r"(addr));
  asm volatile("eret  \n"::);
}

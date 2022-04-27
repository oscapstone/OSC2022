#include "printf.h"
#include "scheduler.h"
#include "malloc.h"

#define THREAD_SP_SIZE 1024

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
  printf("init thread\n\r");
}

void idle_thread(void){
  write_current((uint64_t)dequeue(&run_queue));
  while (1){
    task *cur = get_current();
    printf("Thread id: %d\n\r", cur->pid);
    schedule();
    kill_zombies();
  }
}

void task_create(tread_func func){
  task *new_task = malloc(sizeof(task));
  new_task->lr = (uint64_t)func;
  new_task->next = NULL;
  new_task->pid = pid++;
  char *addr = malloc(THREAD_SP_SIZE);
  new_task->fp = (uint64_t)addr;
  new_task->sp = (uint64_t)addr;
  new_task->sp_addr = (uint64_t)addr;
  new_task->state = RUNNING;
  enqueue(&run_queue, new_task);
  return;
}

void schedule(){
  task *cur = get_current();
  cur->next = NULL;
  if(cur->state == RUNNING)
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
  }
}

void enqueue(task **queue, task *new_task){
  task *cur = *queue;
  task *prev = cur;
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

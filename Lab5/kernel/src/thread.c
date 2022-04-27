#include "thread.h"
#include "string.h"
#include "io.h"
#include "memory.h"
#include "printf.h"
#include "utils.h"

void foo() {
  for (int i = 0; i < 4; ++i) {
    //printf("Thread id: %d, %d\r\n", current_thread()->tid, i);
    print_s("Thread id: ");
    print_i(current_thread()->pid);
    print_s("\r\n");
    delay(10);
    schedule();
  }
  exit();
  return;
}

void thread_test() {
    print_s("asdfasdf\r\n");
    //printf("malloc succ\n");
  thread_info *idle_t = thread_create(0);
  asm volatile("msr tpidr_el1, %0\n" ::"r"((uint64_t)idle_t));
  for (int i = 0; i < 5; ++i) {
    print_i(i);
    print_s("\r\n");
    thread_create(foo);
  }
  idle();
}

void thread_init() {
  run_queue.head = 0;
  run_queue.tail = 0;
  thread_cnt = 0;
}

thread_info *thread_create(void (*func)()) {
  thread_info *thread = (thread_info *)malloc(sizeof(thread_info));
  thread->pid = thread_cnt++;
  thread->status = THREAD_READY;
  thread->next = 0;
  thread->kernel_stack_base = (uint64_t)malloc(STACK_SIZE);
  thread->user_stack_base = 0;
  thread->user_program_base =
      USER_PROGRAM_BASE + thread->pid * USER_PROGRAM_SIZE;
  thread->context.fp = thread->kernel_stack_base + STACK_SIZE;
  thread->context.lr = (uint64_t)func;
  thread->context.sp = thread->kernel_stack_base + STACK_SIZE;
  run_queue_push(thread);
  return thread;
}


void run_queue_push(thread_info *thread) {
  if (run_queue.head == 0) {
    run_queue.head = run_queue.tail = thread;
  } else {
    // printf("%p\n", run_queue.tail);
    // printf("%p\n", run_queue.tail->next);
    run_queue.tail->next = thread;
    run_queue.tail = thread;
  }
}

void schedule() {
  //print_s("scheduling\r\n");
  if (run_queue.head == 0) {
    return;
  }
  if (run_queue.head == run_queue.tail) {  // idle thread
    // uart_puts("??\r\n");
    free(run_queue.head);
    run_queue.head = run_queue.tail = 0;
    thread_cnt = 0;
    return;
  }

  do {
    //print_s("lalala\r\n");
    run_queue.tail->next = run_queue.head;
    run_queue.tail = run_queue.head;
    run_queue.head = run_queue.head->next;
    run_queue.tail->next = 0;
  } while (run_queue.head->status != THREAD_READY);
  // unsigned long sp_addr;
  // asm volatile("ldr %0, [sp]\n":"=r"(sp_addr):);
  // printf("[schedule]svc, sp: %x\n", sp_addr);
  //print_s("switching\r\n");
  switch_to((uint64_t)get_current(), (uint64_t)&run_queue.head->context);
}

void idle() {
  while (1) {
    kill_zombies();
    schedule();
    //print_s("reee\r\n");
  }
}

void exit() {
  // read this?????
  thread_info *cur = current_thread();
  cur->status = THREAD_DEAD;
  schedule();
}

void kill_zombies() {
  if (run_queue.head == 0) return;
  for (thread_info *ptr = run_queue.head; ptr->next != 0; ptr = ptr->next) {
    for (thread_info *cur = ptr->next; cur != 0 && cur->status == THREAD_DEAD;) {
      thread_info *tmp = cur->next;
      // printf("find dead thread %d\n", cur->tid);
      free((void *)cur);
      ptr->next = tmp;
      cur = tmp;
    }
    if (ptr->next == 0) {
      run_queue.tail = ptr;
      break;
    }
  }
}

thread_info *current_thread() {
  thread_info *ptr;
  asm volatile("mrs %0, tpidr_el1\n" : "=r"(ptr) :);
  return ptr;
}
#include "thread.h"

#include "alloc.h"
#include "cpio.h"
#include "exception.h"
#include "printf.h"
#include "utils.h"
#include "timer.h"
int schedule_cnt = 0;
void foo() {
  // core_timer_enable();
  for (int i = 0; i < 5; i++) {
    printf("Thread id: %d, %d\r\n", get_current()->tid, i);
    delay(100000000);
    // schedule();
    // printf("After foo schedule\r\n");
  }
  // core_timer_disable();
  exit();
  return;
}

void thread_test1() {
  thread_info *idle_t = thread_create(0);
  asm volatile("msr tpidr_el1, %0\n" ::"r"((uint64_t)idle_t));
  for (int i = 0; i < 4; i++) {
    thread_create(foo);
  }
  core_timer_enable();
  idle();
}

void user_test() {
  const char *argv[] = {"argv_test", "-o", "arg2", 0};
  exec("my_test", argv);
}

void user_test2() {
  const char *argv[] = {"argv_test", "-o", "arg2", 0};
  exec("my_test2", argv);
}

void thread_test2() {
  thread_info *idle_t = thread_create(0);
  asm volatile("msr tpidr_el1, %0\n" ::"r"((uint64_t)idle_t));
  thread_create(user_test);
  thread_create(user_test2);
  idle();
}

void thread_init() {
  run_queue.head = 0;
  run_queue.tail = 0;
  thread_cnt = 0;
}

thread_info *thread_create(void (*func)()) {
  thread_info *thread = (thread_info *)malloc(sizeof(thread_info));
  void *kernel_stack_base = malloc(STACK_SIZE);
  thread->context.fp = (uint64_t)kernel_stack_base + STACK_SIZE;
  thread->context.lr = (uint64_t)func;
  thread->context.sp = (uint64_t)kernel_stack_base + STACK_SIZE;
  thread->user_sp = 0;
  thread->tid = thread_cnt++;
  thread->status = ALIVE;
  thread->next = 0;
  thread->user_stack_base = 0;
  thread->user_program_base = USER_PROGRAM_BASE + thread->tid * USER_PROGRAM_SIZE;
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
  if (run_queue.head == 0) {
    return;
  }
  if (run_queue.head == run_queue.tail) {  // idle thread
    // printf("run_queue.head == run_queue.tail\r\n");
    free(run_queue.head);
    run_queue.head = run_queue.tail = 0;
    thread_cnt = 0;
    enable_interrupt(); // need uart interrupt when go back to shell
    return;
  }

  do {
    // printf("schedule\n");
    run_queue.tail->next = run_queue.head;
    run_queue.tail = run_queue.head;
    run_queue.head = run_queue.head->next;
    run_queue.tail->next = 0;
  } while (run_queue.head->status != ALIVE);
  // unsigned long sp_addr;
  // asm volatile("ldr %0, [sp]\n":"=r"(sp_addr):);
  // printf("[schedule]svc, sp: %x\n", sp_addr);
  // printf("current_thread()->tid = %d\n",current_thread()->tid);
  // printf("run_queue.head->tid = %d\n",run_queue.head->tid);
  // printf("current_thread()->lr = %d\n",current_thread()->context.lr);
  // printf("run_queue.head->context->lr=%d\n",run_queue.head->context.lr);
  enable_interrupt();
  switch_to(get_current(), run_queue.head);
  // printf("FINISH SCHEDULE_CNT = %d\n", ++schedule_cnt);

}

void idle() {
  // printf("idle\n");
  while (1) {
    kill_zombies();
    schedule();
    if (run_queue.head == 0) break;
  }
  printf("finish\n");
}

void exit() {
  // printf("==============exit=============\n");
  thread_info *cur = get_current();
  cur->status = DEAD;
  schedule();
  // printf("After exit schedule\r\n");

}

void kill_zombies() {
  if (run_queue.head == 0) return;
  for (thread_info *ptr = run_queue.head; ptr->next != 0; ptr = ptr->next) {
    for (thread_info *cur = ptr->next; cur != 0 && cur->status == DEAD;) {
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

void exec(const char *program_name, const char **argv) {
  thread_info *cur = get_current();

  uint64_t spsr_el1 = 0x0;  // EL0t with interrupt enabled
  uint64_t target_addr = 0x30010000+0x1000000*cur->tid;
  uint64_t target_sp = 0x30000000+0x100000*cur->tid;
  printf("target_addr:%p\n", target_addr);
  printf("target_sp:%p\n", target_sp);
  asm volatile("msr spsr_el1, %0" : : "r"(spsr_el1));
  asm volatile("msr elr_el1, %0" : : "r"(target_addr));
  asm volatile("msr sp_el0, %0" : : "r"(target_sp));
  cpio_load_user_program(program_name, target_addr);
  core_timer_enable();
  // asm volatile("mrs x3, sp_el0");
  // asm volatile("ldr x0, [x3, 0]");
  // asm volatile("ldr x1, [x3, 8]");
  asm volatile("eret");
}
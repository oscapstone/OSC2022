#include "thread.h"
#include "timer.h"
#include "alloc.h"
#include "cpio.h"
#include "exception.h"
#include "printf.h"
#include "utils.h"
#include "shell.h"

void foo() {
  for (int i = 0; i < 4; ++i) {
    printf("Thread id: %d, %d\r\n", get_current()->pid, i);
    delay(1000000);
    schedule();
  }
  exit();
  return;
}

void user_test1() {
  const char *argv[] = {"argv_test", "-o", "arg2", 0};
  exec("my_test", argv);
}

void user_test2() {
  const char *argv[] = {"argv_test", "-o", "arg2", 0};
  exec("my_test2", argv);
}
void user_test3() {
  const char *argv[] = {"argv_test", "-o", "arg2", 0};
  exec("fork_test", argv);
}

void user_test4() {
  const char *argv[] = {"argv_test", "-o", "arg2", 0};
  exec("vfs1.img", argv);
}

void user_test5() {
  const char *argv[] = {"argv_test", "-o", "arg2", 0};
  exec("vm.img", argv);
}

void thread_test1() { // thread test
  thread_info *idle_t = thread_create(0);
  asm volatile("msr tpidr_el1, %0\n" ::"r"((uint64_t)idle_t));
  thread_create(user_test1);
  thread_create(user_test2);
  idle();
}

void thread_test2() { // fork test
  thread_info *idle_t = thread_create(0);
  asm volatile("msr tpidr_el1, %0\n" ::"r"((uint64_t)idle_t));
  thread_create(user_test3);
  idle();
}

void thread_test3() { //vedio player1 test
  thread_info *idle_t = thread_create(0);
  asm volatile("msr tpidr_el1, %0\n" ::"r"((uint64_t)idle_t));
  thread_create(user_test4);
  idle();
}

void thread_test4() { //vedio player1 test
  thread_info *idle_t = thread_create(0);
  asm volatile("msr tpidr_el1, %0\n" ::"r"((uint64_t)idle_t));
  thread_create(user_test5);
  idle();
}

void thread_init() {
  run_queue.head = 0;
  run_queue.tail = 0;
  thread_cnt = 0;
}

thread_info *thread_create(void (*func)()) {
  // printf("create thread pid = %d\n",thread_cnt);
  thread_info *thread = (thread_info *)malloc(sizeof(thread_info));
  thread->pid = thread_cnt++;
  thread->status = THREAD_READY;
  thread->next = 0;
  thread->kernel_stack_base = (uint64_t)malloc(STACK_SIZE);
  thread->user_stack_base = (uint64_t)malloc(STACK_SIZE);
  thread->user_program_base =
      USER_PROGRAM_BASE + thread->pid * USER_PROGRAM_SIZE;
  thread->context.fp = thread->kernel_stack_base + STACK_SIZE;
  thread->context.lr = (uint64_t)func;
  thread->context.sp = thread->kernel_stack_base + STACK_SIZE;
  for (int i = 0; i < FD_MAX; ++i) thread->fd_table.files[i] = 0;
  thread->fd_table.files[0] = stdin;
  thread->fd_table.files[1] = stdout;
  thread->fd_table.files[2] = stderr;
  run_queue_push(thread);
  return thread;
}

void schedule() {
  // printf("[schdule]\n");
  if (run_queue.head == 0) {
    // printf("no thread\n");
    enable_uart_interrupt();
    core_timer_disable();
    enable_interrupt();
    return;
  }
  if (run_queue.head == run_queue.tail) {  // idle thread
    // printf("left idle thread\n");
    free(run_queue.head);
    run_queue.head = run_queue.tail = 0;
    thread_cnt = 0;
    enable_interrupt();  // need uart interrupt when go back to shell
    return;
  }

  do {
    run_queue.tail->next = run_queue.head;
    run_queue.tail = run_queue.head;
    run_queue.head = run_queue.head->next;
    run_queue.tail->next = 0;
  } while (run_queue.head->status != THREAD_READY);
  // printf("get_current()->pid = %d\n",get_current()->pid);
  // printf("run_queue.head->pid = %d\n",run_queue.head->pid);
  enable_interrupt();
  switch_to(get_current(), run_queue.head);
}

void idle() {
  // printf("[idle]\n");

  while (1) {
    kill_zombies();
    handle_fork();
    schedule();
    if (run_queue.head == 0) break;    //blcok here if any thread exist=
  }
}

void exit() {
  thread_info *cur = get_current();
  cur->status = THREAD_DEAD;
  schedule();
}

void run_queue_push(thread_info *thread) {
  if (run_queue.head == 0) {
    run_queue.head = run_queue.tail = thread;
  } else {
    run_queue.tail->next = thread;
    run_queue.tail = thread;
  }
}

void kill_zombies() {
  if (run_queue.head == 0) return;
  for (thread_info *ptr = run_queue.head; ptr->next != 0; ptr = ptr->next) {
    for (thread_info *cur = ptr->next;
         cur != 0 && (cur->status == THREAD_DEAD);) {
      thread_info *tmp = cur->next;
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

  uint64_t user_sp = cur->user_stack_base + STACK_SIZE;
  cur->user_program_size = cpio_load_user_program(program_name, cur->user_program_base);
  // printf("cur->pid = %d, cur->user_program_base = %x\n",cur->pid,cur->user_program_base);

  // return to user program
  uint64_t spsr_el1 = 0x0;  // EL0t with interrupt enabled
  uint64_t target_addr = cur->user_program_base;
  uint64_t target_sp = user_sp;
  core_timer_enable();
  asm volatile("msr spsr_el1, %0" : : "r"(spsr_el1));
  asm volatile("msr elr_el1, %0" : : "r"(target_addr));
  asm volatile("msr sp_el0, %0" : : "r"(target_sp));
  asm volatile("eret");
}

void fork(uint64_t sp) {
  run_queue.head->status = THREAD_FORK;
  run_queue.head->trap_frame_addr = sp;
  schedule();
  trap_frame_t *trap_frame = (trap_frame_t *)(get_current()->trap_frame_addr);
  trap_frame->x[0] = run_queue.head->child_pid;
}

void handle_fork() {
  // printf("[handle_fork]\n");
  for (thread_info *ptr = run_queue.head->next; ptr != 0; ptr = ptr->next) {
    if ((ptr->status) == THREAD_FORK) {
      thread_info *child = thread_create(0);
      create_child(ptr, child);
      ptr->status = THREAD_READY;
      child->status = THREAD_READY;
    }
  }
}

void create_child(thread_info *parent, thread_info *child) {
  // printf("[create_child]\n");
  disable_interrupt();
  child->user_stack_base = (uint64_t)malloc(STACK_SIZE);
  child->user_program_size = parent->user_program_size;
  parent->child_pid = child->pid;
  child->child_pid = 0;

  char *src, *dst;
  // copy saved context in thread info
  src = (char *)&(parent->context);
  dst = (char *)&(child->context);
  for (uint32_t i = 0; i < sizeof(cpu_context); ++i, ++src, ++dst) {
    *dst = *src;
  }
  // copy kernel stack
  src = (char *)(parent->kernel_stack_base);
  dst = (char *)(child->kernel_stack_base);
  for (uint32_t i = 0; i < STACK_SIZE; ++i, ++src, ++dst) {
    *dst = *src;
  }
  // copy user stack
  src = (char *)(parent->user_stack_base);
  dst = (char *)(child->user_stack_base);
  for (uint32_t i = 0; i < STACK_SIZE; ++i, ++src, ++dst) {
    *dst = *src;
  }
  // copy user program
  src = (char *)(parent->user_program_base);
  dst = (char *)(child->user_program_base);
  for (uint32_t i = 0; i < parent->user_program_size; ++i, ++src, ++dst) {
    *dst = *src;
  }

  // set correct address for child
  uint64_t kernel_stack_base_dist =
      child->kernel_stack_base - parent->kernel_stack_base;
  uint64_t user_stack_base_dist =
      child->user_stack_base - parent->user_stack_base;
  uint64_t user_program_base_dist =
      child->user_program_base - parent->user_program_base;

  child->context.fp += kernel_stack_base_dist;
  child->context.sp += kernel_stack_base_dist;
  child->trap_frame_addr = parent->trap_frame_addr + kernel_stack_base_dist;
  trap_frame_t *trap_frame = (trap_frame_t *)(child->trap_frame_addr);
  trap_frame->x[29] += user_stack_base_dist;    // fp (x29)
  trap_frame->x[30] += user_program_base_dist;  // lr (x30)
  trap_frame->x[32] += user_program_base_dist;  // elr_el1
  trap_frame->x[33] += user_stack_base_dist;    // sp_el0
  enable_interrupt();
}
void kill (int kill_pid)
{
  if (run_queue.head == 0) return;
  for (thread_info *ptr = run_queue.head; ptr->next != 0; ptr = ptr->next) {
    if(ptr->pid == kill_pid){
      printf("Kill pid = %d\n",kill_pid);
      ptr->status = THREAD_DEAD;
      schedule();
      return;
    }
  }
  printf("pid = %d not exist\n",kill_pid);

}

struct file *thread_get_file(int fd) {
  thread_info *cur = get_current();
  return cur->fd_table.files[fd];
}

int thread_register_fd(struct file *file) {
  if (file == 0) return -1;
  thread_info *cur = get_current();
  // find next available fd
  for (int fd = 3; fd < FD_MAX; ++fd) {
    if (cur->fd_table.files[fd] == 0) {
      cur->fd_table.files[fd] = file;
      return fd;
    }
  }
  return -1;
}

int thread_clear_fd(int fd) {
  if (fd < 0 || fd >= FD_MAX) return -1;
  thread_info *cur = get_current();
  cur->fd_table.files[fd] = 0;
  return 1;
}
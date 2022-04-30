#include "thread.h"
#include "timer.h"
#include "string.h"
#include "io.h"
#include "memory.h"
#include "printf.h"
#include "utils.h"
#include "kernel.h"

void thread_init() {
  run_queue.head = 0;
  run_queue.tail = 0;
  thread_cnt = 0;

  from_kernel = 1;
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

  // no other thread to run
  if (run_queue.head == 0) {
    printf("nothing to run\n");
    run_shell();
    return;
  }
  // check if there's any other thread to run
  if (run_queue.head == run_queue.tail) {  // idle thread
    free(run_queue.head);
    run_queue.head = run_queue.tail = 0;
    thread_cnt = 0;
    return;
  }

  do {
    //print_s("dfdf\r\n");
    run_queue.tail->next = run_queue.head;
    run_queue.tail = run_queue.head;
    run_queue.head = run_queue.head->next;
    run_queue.tail->next = 0;
  } while (run_queue.head->status != THREAD_READY);
  //unsigned long sp_addr;
  //asm volatile("ldr %0, [sp]\n":"=r"(sp_addr):);
  //print_i(get_current());
  //printf("[schedule]svc, sp: %x\n", sp_addr);
  //uint64_t cn;
  //  asm volatile("mrs %0, cntpct_el0" : "=r"(cn));
  //  print_s("\r\n");
  //  print_i(cn);
  //  print_s("time counter\r\n");

  //enable_interrupt();
  //plan_next_interrupt_tval(SCHEDULE_TVAL);
  plan_next_interrupt_tval(SCHEDULE_TVAL);
  enable_interrupt();
  switch_to((uint64_t)get_current(), (uint64_t)&run_queue.head->context);
}

void timer_schedule() {
  //print_s("scheduling\r\n");

  // no other thread to run
  if (run_queue.head == 0) {
    print_s("nothing to run");
    return;
  }
  // check if there's any other thread to run
  //if (run_queue.head == run_queue.tail) {  // idle thread
  //  free(run_queue.head);
  //  run_queue.head = run_queue.tail = 0;
  //  thread_cnt = 0;
  //  return;
  //}

  do {
    //print_s("dfdf\r\n");
    run_queue.tail->next = run_queue.head;
    run_queue.tail = run_queue.head;
    run_queue.head = run_queue.head->next;
    run_queue.tail->next = 0;
  } while (run_queue.head->status != THREAD_READY);
}

void idle() {
  while (1) {
    kill_zombies();
    schedule();
  }
}

void exit() {
  //disable_interrupt();
  thread_info *cur = current_thread();
  cur->status = THREAD_DEAD;
  //cur->context.lr = 0;
  //print_s("\r\nexit: ");
  //print_i(cur->pid);
  //print_s(", thread calls exit!!!!!!!!!!!!!!\r\n");
  schedule();
  //timer_schedule();
  //run_queue.head = idle_t;
  //printf
  //enable_interrupt();
  //switch_to(get_current(), idle_t);
  //while(1){}
  //switch_to(get_current(), idle_t);
}

void kill_zombies() {
  //disable_interrupt();
  //printf("killing zombies\n");
  if (run_queue.head == 0) return;
  //print_s("killing zombies 2222\r\n");
  for (thread_info *ptr = run_queue.head; ptr->next != 0; ptr = ptr->next) {
    //print_s("finding zombie\r\n");
    //printf("[pid:%d, status:%d] -> ", ptr->pid, ptr->status);
    for (thread_info *cur = ptr->next; cur != 0 && cur->status == THREAD_DEAD; /**/ ) {
      thread_info *tmp = cur->next;
      // printf("find dead thread %d\n", cur->tid);
      printf("find dead thread, pid: %d\n", cur->pid);
      free((void *)cur);
      ptr->next = tmp;
      cur = tmp;
    }
    if (ptr->next == 0) {
      run_queue.tail = ptr;
      break;
    }
  }
  //printf("[pid:%d] -> ", run_queue.head->pid);
  //printf("\n");
  //enable_interrupt();
}

thread_info *current_thread() {
  thread_info *ptr;
  asm volatile("mrs %0, tpidr_el1\n" : "=r"(ptr) :);
  return ptr;
}

void timer_schedular_init(){
  //thread_info *idle_t = thread_create(0);
  //asm volatile("msr tpidr_el1, %0\n" ::"r"((uint64_t)idle_t));
  //core_timer_enable(SCHEDULE_TVAL);
}

void timer_schedular_handler(){
  //core_timer_disable();
  //print_s("timer_schedular_handler\r\n");
  
  timer_schedule();
  kill_zombies();
  plan_next_interrupt_tval(SCHEDULE_TVAL);
  //print_s("set next interrupt\r\n");
}

void save_thread_info(exception_frame_t* ef){

  thread_info* curr = (thread_info*)ef->tpidr_el1;
  //for(int i = 0; i<31; i++){
  //  curr->context.x[i] ef->x[i];
  //}
  curr->context.x19 = ef->x[19];
  curr->context.x20 = ef->x[20];
  curr->context.x21 = ef->x[21];
  curr->context.x22 = ef->x[22];
  curr->context.x23 = ef->x[23];
  curr->context.x24 = ef->x[24];
  curr->context.x25 = ef->x[25];
  curr->context.x26 = ef->x[26];
  curr->context.x27 = ef->x[27];
  curr->context.x28 = ef->x[28];
  
  //curr->context.fp = curr->context.fp;
  curr->context.lr = ef->lr;
  curr->context.sp = ef->sp;
}

void load_thread_info(thread_info * curr, exception_frame_t* ef){
  //for(int i = 0; i<31; i++){
  //  ef->x[i] = curr->context.x[i];
  //}
   ef->x[19] = curr->context.x19;
   ef->x[20] = curr->context.x20;
   ef->x[21] = curr->context.x21;
   ef->x[22] = curr->context.x22;
   ef->x[23] = curr->context.x23;
   ef->x[24] = curr->context.x24;
   ef->x[25] = curr->context.x25;
   ef->x[26] = curr->context.x26;
   ef->x[27] = curr->context.x27;
   ef->x[28] = curr->context.x28;
  
  //curr->context.fp = ((thread_info*)ef->tpidr_el1)->context.fp;
  //curr->context.fp = curr->context.fp;
  ef->tpidr_el1 = curr;
  ef->lr = curr->context.lr;
  ef->sp = curr->context.sp;
}




void fork(uint64_t sp) {
  run_queue.head->status = THREAD_FORK;
  run_queue.head->trap_frame_addr = sp;
  schedule();
  trap_frame_t *trap_frame = (trap_frame_t *)(get_current()->trap_frame_addr);
  trap_frame->x[0] = run_queue.head->child_pid;
}

void handle_fork() {
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
}


void foo() {
  for (int i = 0; i < 4; ++i) {
    //printf("Thread id: %d, %d\r\n", current_thread()->tid, i);
    print_s("Thread id: ");
    print_i(current_thread()->pid);
    print_s("\r\n");
    delay(100000000);
    schedule();
  }
  exit();
  return;
}

void foo2(){
  
  for (int i = 1; i <= 5; ++i) {
    //core_timer_enable(SCHEDULE_TVAL);
    //printf("Thread id: %d, %d\r\n", current_thread()->tid, i);
    uint64_t cn;
    asm volatile("mrs %0, cntpct_el0" : "=r"(cn));
    print_s("\r\n");
    print_i(i);
    print_s(",foo2 Thread id: ");
    print_i(current_thread()->pid);
    print_s("\r\n");
    delay(100000000);
  }
  //print_s("\n\n\n\ndone!!!!!!\r\n");
  exit();
}
void foo3(){
  while(1){
    printf("foo3\n");
    delay(100000000);
  }
}

//void thread(callback func)

void thread_test() {
  // I'm not sure why you need following lines
  thread_info *idle_t = thread_create(foo3);
  asm volatile("msr tpidr_el1, %0\n" ::"r"((uint64_t)idle_t));
  //for (int i = 0; i < 5; ++i) {
  //  print_i(i);
  //  print_s("\r\n");
  //  thread_create(foo);
  //}
  thread_create(exec);
  idle();
}

void thread_timer_test(){
  // scheduling using timer interrupt
  print_s("timer schedular test\r\n");
  //timer_schedular_init();
  idle_t = thread_create(foo3);
  asm volatile("msr tpidr_el1, %0\n" ::"r"((uint64_t)idle_t));

  for (int i = 0; i <5; ++i) {
    print_i(i);
    print_s("\r\n");
    thread_create(foo2);
  }
  //timer_schedular_init();
  bp("start timer\r\n");
  core_timer_enable(SCHEDULE_TVAL);
  enable_interrupt();
  //idle_thread();
}



void exec() {
    //print_s(args);
    uint64_t spsr_el1 = 0x0;  // EL0t with interrupt enabled, PSTATE.{DAIF} unmask (0), AArch64 execution state, EL0t
    uint64_t target_addr = 0x30100000; // load your program here
    uint64_t target_sp = 0x21000000;

    //cpio_load_user_program("user_program.img", target_addr);
    cpio_load_user_program("syscall.img", target_addr);
    //core_timer_enable();
    //bp("bp1");
    asm volatile("msr spsr_el1, %0" : : "r"(spsr_el1)); // set PSTATE, executions state, stack pointer
    asm volatile("msr elr_el1, %0" : : "r"(target_addr)); // link register at 
    asm volatile("msr sp_el0, %0" : : "r"(target_sp));
    asm volatile("eret"); // eret will fetch spsr_el1, elr_el1.. and jump (return) to user program.
                          // we set the register manually to perform a "jump" or switchning between kernel and user space.
}

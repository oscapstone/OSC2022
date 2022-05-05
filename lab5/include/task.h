#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#define MAX_TASKS 256
#define MAX_PRIORITY 30
#define TIME_QUANTUM_MSEC 10

typedef enum { eFree=0, eRunning, eReady, eBlocked, eTerminated} eTaskState;

struct el1_regs {
  uint64_t x19;
  uint64_t x20;
  uint64_t x21;
  uint64_t x22;
  uint64_t x23;
  uint64_t x24;
  uint64_t x25;
  uint64_t x26;
  uint64_t x27;
  uint64_t x28;
  uint64_t fp;
  uint64_t lr;
  uint64_t sp;
  uint64_t spsr_el1;
  uint64_t elr_el1;
  uint64_t esr_el1;
  uint64_t sp_el0;
};

struct el0_regs {
  uint64_t general_purpose[31];
  uint64_t elr_el1;
  uint64_t spsr_el1;
  uint64_t padding;
};

struct taskControlBlock {
  uint32_t pid;
  uint32_t priority;
  eTaskState state;
  struct el1_regs regs;
  void* kernelStackPage;
  void* userStackPage;
  int userStackExp;
  int exePageExp;
  void* exePage;
};

int getCurrentPid();
struct taskControlBlock* addTask(void (*func)(), int priority);

void schedule();
void startScheduler();
void reset_timer();
void timerInterruptHandler();
void startInEL0(uint64_t pc);


int syscall_getpid();
void syscall_exit();
void disable_preempt();
void enable_preempt();

extern void context_switch(struct taskControlBlock *old_tcb,
                           struct taskControlBlock *new_tcb,
                           uint64_t reg_offset);


#endif

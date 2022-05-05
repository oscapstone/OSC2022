#include <stddef.h>
#include <string.h>
#include "task.h"
#include "mem.h"
#include "list.h"
#include "textio.h"
#include "timer.h"
#include "except.h"

struct taskControlBlock tasks[MAX_TASKS];
int taskCount;
struct taskControlBlock *currentTask;
int preemptable;

struct list readylist;
struct listItem taskListItem[MAX_TASKS];

int getCurrentPid() { return currentTask->pid; }

struct taskControlBlock* addTask(void (*func)(), int priority) {
  int freeTaskIdx = -1;
  for (int i = 1; i < MAX_TASKS; i++) {
    if (tasks[i].state == eFree) {
      freeTaskIdx = i;
      break;
    }
  }
  if (freeTaskIdx == -1) {
    kprintf("[KTask] Cannot create more task\n");
    return NULL;
  }

  taskCount++;
  struct taskControlBlock *tsk = &tasks[freeTaskIdx];
  struct listItem *elm = &taskListItem[freeTaskIdx];
  tsk->pid = freeTaskIdx;
  tsk->priority = priority;
  tsk->state = eReady;
  tsk->userStackPage = NULL;
  tsk->kernelStackPage = getFreePage();
  tsk->userStackExp = -1;
  tsk->exePageExp = 0;
  tsk->regs.lr = (uint64_t)func;
  tsk->regs.sp = (uint64_t)tsk->kernelStackPage + FRAME_SIZE;

  elm->size = sizeof(struct taskControlBlock);
  elm->data = tsk;
  listAppend(&readylist, elm);
  return tsk;
}


static void killZombies() {
  // kprintf("[KTask] Killing Zombies...\n");
  disable_preempt();
  for (int i = 1; i < MAX_TASKS; i++) {
    if (tasks[i].state == eTerminated) {
      kprintf("[KTask] Killing %d\n", i);
      returnPage(tasks[i].kernelStackPage);
      if (tasks[i].userStackPage != NULL) {
        int exp = tasks[i].userStackExp;
        int idx = ((uint64_t)tasks[i].userStackPage - MEMORY_BASE) / FRAME_SIZE;
        for (int j = 0; j < (1<<exp); j++)
          deallocate_frame(idx+j);
      }
      if (tasks[i].exePage != NULL) {
        int exp = tasks[i].exePageExp;
        int idx = ((uint64_t)tasks[i].exePage - MEMORY_BASE) / FRAME_SIZE;
        for (int j = 0; j < (1<<exp); j++)
          deallocate_frame(idx+j);
      }
      memset(&tasks[i], 0, sizeof(tasks[i]));
      kprintf("[KTask] Task No.%d is recycled.\n", i);
    }
  }
  enable_preempt();
}

static void idle() {
  preemptable = 1;
  enable_core_timer();
  reset_timer();
  
  kprintf("[KTask] Start Scheduler\n");
  while (1) {
    killZombies();
    schedule();
  }
}

int initIdleTask() {
  taskCount++;
  tasks[0].pid = 0;
  tasks[0].priority = MAX_PRIORITY-1;
  tasks[0].state = eRunning;
  tasks[0].kernelStackPage = getFreePage();
  tasks[0].userStackPage = NULL;
  tasks[0].regs.sp = (uint64_t)tasks[0].kernelStackPage + FRAME_SIZE;
  tasks[0].regs.lr = (uint64_t)&idle;
  taskListItem[0].data = &tasks[0];
  taskListItem[0].size = sizeof(tasks[0]);
  currentTask = &tasks[0];
  return currentTask->pid;
}

void reset_timer() {
  uint64_t cpu_freq = get_cpu_freq();
  // set_relative_timer(cpu_freq * TIME_QUANTUM_MSEC / 1000);
  set_relative_timer(cpu_freq >> 5);
}

void startScheduler() {
  initIdleTask();
  
  uint64_t regsp = tasks[0].regs.sp;
  uint64_t reglr = tasks[0].regs.lr;
  asm volatile("mov lr, %0\n\t"
               "mov sp, %1\n\t"
               "ret"
               :: "r" (reglr), "r" (regsp));
}

void schedule() {
  // critical section begin
  preemptable = 0;
  if (readylist.itemCount > 0) {
    struct listItem *fstItem = readylist.first;
    struct taskControlBlock *newTsk = fstItem->data;
    struct taskControlBlock *oldTsk = currentTask;
    newTsk->state = eRunning;
    listRemoveItem(&readylist, fstItem);
    if (oldTsk->state == eRunning) {
      oldTsk->state = eReady;
      listAppend(&readylist, &taskListItem[oldTsk->pid]);
    }
    currentTask = newTsk;
    // kprintf("[KTask] switch from %d to %d\n", oldTsk->pid, newTsk->pid);
    context_switch(oldTsk, newTsk, __builtin_offsetof(struct taskControlBlock, regs));
  }
  preemptable = 1;
  // critical section end
}

void timerInterruptHandler() {
  reset_timer();
  if (!preemptable) {
    // kprintf("[K] nothing happend\n");
    return;
  }
  enable_irq();
  schedule();
  disable_irq();
}

void startInEL0(uint64_t pc) {
  disable_irq();
  currentTask->regs.spsr_el1 = 0x340; // enable irq (disable 3C0)
  currentTask->regs.esr_el1 = pc;
  currentTask->userStackPage = getContFreePage(4, &currentTask->userStackExp);
  uint64_t spel0 = (uint64_t)currentTask->userStackPage + 4*FRAME_SIZE;
  kprintf("[K] Move process no.%d to userspace.\n", currentTask->pid);
  asm volatile("msr spsr_el1, %0\n\t"
               "msr elr_el1, %1\n\t"
               "msr sp_el0, %2\n\t"
               "mov sp, %3\n\t" // clear kernel stack
               "eret"
               ::
                "r" (currentTask->regs.spsr_el1),
                "r" (currentTask->regs.esr_el1),
                "r" (spel0),
                "r" (currentTask->kernelStackPage + FRAME_SIZE));
}

int syscall_getpid() { return getCurrentPid(); }

void syscall_exit() {
  kprintf("[KTask] Exit %d\n", currentTask->pid);
  currentTask->state = eTerminated;
  while (1) asm volatile("nop");
}

void enable_preempt() { preemptable = 1; }
void disable_preempt() { preemptable = 0; }


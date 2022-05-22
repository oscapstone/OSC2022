#include <stddef.h>
#include <string.h>
#include "mmu.h"
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
  struct listItem *tskListElm = &taskListItem[freeTaskIdx];
  tsk->pid = freeTaskIdx;
  tsk->priority = priority;
  tsk->state = eReady;

  uint64_t vKernelStack = (uint64_t)getFreePage() + VA_START;
  tsk->mem.kernel_pages[0].vBegin = (void*)vKernelStack;
  tsk->mem.kernel_pages[0].exp = 0;
  tsk->mem.kernel_pages_cnt = 1;

  tsk->mem.user_pages_cnt = 0;
  tsk->mem.pgd = 0;
  
  tsk->regs.sp = vKernelStack + FRAME_SIZE;
  tsk->regs.lr = (uint64_t)func;
  tskListElm->size = sizeof(struct taskControlBlock);
  tskListElm->data = tsk;
  listAppend(&readylist, tskListElm);
  return tsk;
}


static void killZombies() {
  disable_preempt();
  for (int i = 1; i < MAX_TASKS; i++) {
    if (tasks[i].state == eTerminated) {
      kprintf("[KTask] Killing task no.%d\n", i);
      
      int cnt = tasks[i].mem.user_pages_cnt;
      for (int j = 0; j < cnt; j++) {
        int exp = tasks[i].mem.user_pages[i].exp;
        uint64_t kVirt = (uint64_t)translate(&tasks[i], (uint64_t)tasks[i].mem.user_pages[j].vBegin);
        int idx = kVirtualToIndex(kVirt);
        for (int k = 0; k < (1<<exp); k++)
          deallocate_frame(idx+k);
      }
      
      cnt = tasks[i].mem.kernel_pages_cnt;
      for (int j = 0; j < cnt; j++) {
        int exp = tasks[i].mem.kernel_pages[j].exp;
        int idx = kVirtualToIndex((uint64_t)tasks[i].mem.kernel_pages[j].vBegin);
        for (int k = 0; k < (1<<exp); k++)
          deallocate_frame(idx+k);
      }
      /* TODO if we use finer granule we need to reclaiming the kernel page table */
      
      memset(&tasks[i], 0, sizeof(tasks[i]));
      kprintf("[KTask] Task no.%d is killed.\n", i);
    }
  }
  enable_preempt();
}

void idle() {
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

  uint64_t vKernelStack = (uint64_t)getFreePage() + VA_START;
  tasks[0].mem.kernel_pages[0].vBegin = (void*)vKernelStack;
  tasks[0].mem.kernel_pages[0].exp = 0;
  tasks[0].mem.kernel_pages_cnt = 1;

  tasks[0].mem.user_pages_cnt = 0;

  tasks[0].regs.sp = vKernelStack + FRAME_SIZE;
  tasks[0].regs.lr = (uint64_t)idle;
  tasks[0].mem.pgd = EL1_PGD;
  
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
  asm volatile("mov lr, %0\n\t"
               "mov sp, %1\n\t"
               "ret"
               :
               : "r" (tasks[0].regs.lr),
                 "r" (tasks[0].regs.sp));
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
    
    flush_tlb(newTsk->mem.pgd);
    context_switch(oldTsk, newTsk, __builtin_offsetof(struct taskControlBlock, regs));
  }
  preemptable = 1;
  // critical section end
}

void timerInterruptHandler() {
  reset_timer();
  if (!preemptable) {
    return;
  }
  enable_irq();
  schedule();
  disable_irq();
}

/* both pc and sp should be el0 virtual address */
void startInEL0(uint64_t pc, uint64_t sp) {
  disable_irq();
  currentTask->regs.spsr_el1 = 0x340; // enable irq (disable 3C0)
  currentTask->regs.elr_el1 = pc; // where to run in el0
  currentTask->regs.sp_el0 = sp;
  // reset kernel stack
  currentTask->regs.sp = (uint64_t)currentTask->mem.kernel_pages[0].vBegin + FRAME_SIZE;
  kprintf("[K] Move process no.%d to userspace.\n", currentTask->pid);

  flush_tlb(currentTask->mem.pgd);
  
  asm volatile("msr spsr_el1, %0\n\t"
               "msr elr_el1, %1\n\t"
               "msr sp_el0, %2\n\t"
               "mov sp, %3\n\t"
               "eret"
               ::
                "r" (currentTask->regs.spsr_el1),
                "r" (currentTask->regs.elr_el1),
                "r" (currentTask->regs.sp_el0),
                "r" (currentTask->regs.sp));
}

int syscall_getpid() { return (int)currentTask->pid; }

void syscall_exit() {
  kprintf("[KTask] Exit %d\n", currentTask->pid);
  currentTask->state = eTerminated;
  schedule();
}

void enable_preempt() { preemptable = 1; }
void disable_preempt() { preemptable = 0; }


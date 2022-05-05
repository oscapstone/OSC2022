#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "task.h"
#include "mem.h"
#include "except.h"
#include "fork.h"
#include "textio.h"

extern struct taskControlBlock *currentTask;

void forkProcedureForChild() {
  enable_preempt();
  struct el0_regs *trap_frame = (struct el0_regs*)(currentTask->kernelStackPage
                                                   + FRAME_SIZE - sizeof(struct el0_regs));
  trap_frame->general_purpose[0] = 0;
  disable_irq();
  asm volatile("mov sp, %0\n\t"
               "b exit_kernel" :: "r"(trap_frame));
}

int copyCurrentTask() {
  disable_preempt();
  // kprintf("forkProcedureForChild: 0x%lx\n", (uint64_t)forkProcedureForChild);
  struct taskControlBlock *newTask = addTask(forkProcedureForChild, currentTask->priority);
  if (newTask == NULL) return -1;
  // copy user stack and set newTask's sp_el0 before it eret
  uint64_t spel0, elr_el1;
  
  asm volatile("mrs %0, sp_el0\n\t"
               "mrs %1, elr_el1" : "=r"(spel0), "=r"(elr_el1));
  
  uint64_t sp_offset = spel0 - (uint64_t)currentTask->userStackPage;
  newTask->userStackPage = getContFreePage(4, &currentTask->userStackExp);
  
  memcpy((void*)newTask->userStackPage + sp_offset,
         (void*)currentTask->userStackPage + sp_offset,
         (4*FRAME_SIZE)-sp_offset);

  newTask->regs.spsr_el1 = 0x340;
  newTask->regs.elr_el1 = elr_el1;
  newTask->regs.sp = (uint64_t)newTask->kernelStackPage + FRAME_SIZE - sizeof(struct el0_regs);
  newTask->regs.sp_el0 = (uint64_t)newTask->userStackPage + sp_offset;

  // copy trapframe
  struct el0_regs *curTrapFrame = (struct el0_regs*)(currentTask->kernelStackPage + FRAME_SIZE - sizeof(struct el0_regs));
  struct el0_regs *newTrapFrame = (struct el0_regs*)(newTask->kernelStackPage + FRAME_SIZE - sizeof(struct el0_regs));
  memcpy(newTrapFrame, curTrapFrame, sizeof(struct el0_regs));
  
  enable_preempt();
  return newTask->pid;
}

int syscall_fork() {
  return copyCurrentTask();
}

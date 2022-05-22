#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "mmu.h"
#include "task.h"
#include "mem.h"
#include "except.h"
#include "fork.h"
#include "textio.h"


void forkProcedureForChild() {
  enable_preempt();
  struct el0_regs *trap_frame =
    (struct el0_regs*)(currentTask->mem.kernel_pages[0].vBegin
                       + FRAME_SIZE - sizeof(struct el0_regs));
  trap_frame->general_purpose[0] = 0;
  trap_frame->spsr_el1 = currentTask->regs.spsr_el1;
  disable_irq();
  asm volatile("mov sp, %0\n\t"
               "b exit_kernel"
               ::
               "r"(trap_frame));
}


int copyCurrentTask() {
  disable_preempt();
  struct taskControlBlock *newTask = addTask(forkProcedureForChild, currentTask->priority);
  if (newTask == NULL) return -1;
  
  // copy user stack and set newTask's sp_el0 before it eret
  uint64_t spel0, elr_el1;  
  asm volatile("mrs %0, sp_el0\n\t"
               "mrs %1, elr_el1" : "=r"(spel0), "=r"(elr_el1));

  int user_page_cnt = currentTask->mem.user_pages_cnt;
  struct pageBlock *user_pages = currentTask->mem.user_pages;
  
  /* don't have to copy page table since page table is recreated. */
  
  for (int i = 0; i < user_page_cnt; i++) {
    int conti = (1 << user_pages[i].exp);
    void* kvSrc = translate(currentTask, (uint64_t)user_pages[i].vBegin);
    void* kvDst = allocate_user_page(newTask, (uint64_t)user_pages[i].vBegin, conti);
    memcpy(kvDst, kvSrc, conti * FRAME_SIZE);
  }

  /* copy trap frame */
  memcpy(newTask->mem.kernel_pages[0].vBegin + FRAME_SIZE - sizeof(struct el0_regs),
         currentTask->mem.kernel_pages[0].vBegin + FRAME_SIZE - sizeof(struct el0_regs),
         sizeof(struct el0_regs));

  newTask->regs.spsr_el1 = 0x340; // enable irq
  newTask->regs.elr_el1 = elr_el1;
  newTask->regs.sp = (uint64_t)newTask->mem.kernel_pages[0].vBegin + FRAME_SIZE - sizeof(struct el0_regs);
  newTask->regs.sp_el0 = spel0;
  
  enable_preempt();
  return newTask->pid;
}

int syscall_fork() {
  return copyCurrentTask();
}

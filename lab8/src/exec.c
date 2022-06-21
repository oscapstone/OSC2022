#include "cpio.h"
#include "mem.h"
#include "string.h"
#include "stddef.h"
#include "task.h"
#include "exec.h"
#include "textio.h"
#include "mmu.h"
#include "timer.h"

extern uint64_t vInitrdAddr;

int loadExecutable(const char *fname, uint64_t va_exe, uint64_t va_stack) {
  struct cpio_newc_header *p_header = find_file(vInitrdAddr, fname, strlen(fname));
  if (p_header == NULL) return -1;

  /* free original user page first */
  int cnt = currentTask->mem.user_pages_cnt;
  for (int i = 0; i < cnt; i++) {
    int exp = currentTask->mem.user_pages[i].exp;
    uint64_t kVirt = (uint64_t)translate(currentTask, (uint64_t)currentTask->mem.user_pages[i].vBegin);
    int idx = kVirtualToIndex(kVirt);
    for (int j = 0; j < (1<<exp); j++) {
      deallocate_frame(idx+j);
    }
  }
  currentTask->mem.user_pages_cnt = 0;

  /* clear page table */
  cnt = currentTask->mem.kernel_pages_cnt;
  /* don't free the first kernel page which is kernel stack */
  for (int i = 1; i < cnt; i++) {
    int exp = currentTask->mem.kernel_pages[i].exp;
    int idx = kVirtualToIndex((uint64_t)currentTask->mem.kernel_pages[i].vBegin);
    for (int j = 0; j < (1<<exp); j++) {
      deallocate_frame(idx+j);
    }
  }
  currentTask->mem.kernel_pages_cnt = 1;
  currentTask->mem.pgd = 0; // pgd will be reallocate

  uint32_t filesize = cpio_filesize(p_header);
  int pagecnt = filesize / FRAME_SIZE;
  if (filesize % FRAME_SIZE) pagecnt++;

  void* exe_page = allocate_user_page(currentTask, va_exe, pagecnt);
  cpio_read(p_header, 0, exe_page, filesize);

  void* user_stack_page = allocate_user_page(currentTask, va_stack, 4);
  memset(user_stack_page, 0, 4 * FRAME_SIZE);

  flush_tlb(currentTask->mem.pgd);

  kprintf("[K] Load %s into 0x%lx\n", fname, exe_page);
  return va_exe;
}

int syscall_exec(const char *name, char *const argv[]) {
  uint64_t va_exe = 0x0;
  uint64_t va_stack = 0xffffffffb000;
  if (loadExecutable(name, va_exe, va_stack) < 0) {
    return -1;
  }
  struct el0_regs *regs =
    (struct el0_regs*)(currentTask->mem.kernel_pages[0].vBegin
                       + FRAME_SIZE - sizeof(struct el0_regs));
  memset(regs->general_purpose, 0, sizeof(regs->general_purpose));
  regs->elr_el1 = va_exe;
  currentTask->regs.sp_el0 = va_stack + FRAME_SIZE * 4;
  asm volatile("msr sp_el0, %0" :: "r" (currentTask->regs.sp_el0));
  return 0;
}

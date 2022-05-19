#include "cpio.h"
#include "mem.h"
#include "string.h"
#include "stddef.h"
#include "task.h"
#include "exec.h"
#include "textio.h"
#include "mmu.h"

extern uint32_t initrd_addr;

void* loadExecutable(const char *fname) {
  uint64_t addr = VA_START + initrd_addr;
  struct cpio_newc_header *p_header = find_file(addr, fname, strlen(fname));
  if (p_header == NULL) return NULL;
  uint32_t filesize = cpio_filesize(p_header);
  int pagecnt = filesize / FRAME_SIZE;
  if (filesize % FRAME_SIZE) {
    pagecnt++;
  }
  int exp;
  struct taskControlBlock *currentTask = getCurrentTCB();
  currentTask->exePage = getContFreePage(pagecnt, &exp);
  currentTask->exePageExp = exp;
  memset(currentTask->userStackPage, 0, (1<<(currentTask->userStackExp))*FRAME_SIZE);
  asm volatile("msr sp_el0, %0" :: "r"(currentTask->userStackPage+(1<<(currentTask->userStackExp))*FRAME_SIZE));
  cpio_read(p_header, 0, currentTask->exePage, filesize);
  kprintf("[K] Load %s into 0x%lx\n", fname, currentTask->exePage);
  return currentTask->exePage;
}

int syscall_exec(const char *name, char *const argv[]) {
  void *addr = loadExecutable(name);
  if (addr == NULL) return -1;
  struct taskControlBlock *currentTask = getCurrentTCB();
  struct el0_regs *regs = (struct el0_regs*)(currentTask->kernelStackPage + FRAME_SIZE - sizeof(struct el0_regs));
  memset(regs->general_purpose, 0, sizeof(regs->general_purpose));
  regs->elr_el1 = (uint64_t)addr;
  return 0;
}

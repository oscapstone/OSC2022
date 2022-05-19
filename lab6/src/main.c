#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "devtree.h"
#include "mini_uart.h"
#include "mmu.h"
#include "textio.h"
#include "except.h"
#include "mem.h"
#include "task.h"
#include "timer.h"
#include "interface.h"
#include "exec.h"
#include "cpio.h"

#define BUF_LEN 256

uint32_t initrd_addr;
uint32_t initrd_end;

extern uint64_t __kernel_begin;
extern uint64_t __bss_end;
extern uint64_t fdt_begin;
extern uint64_t fdt_size;


void set_initrd_addr(const char* node_name, const char *prop_name, void* value, uint32_t size) {
  // node: chosen prop: linux,initrd-start
  // size 0x00000004
  if (strncmp(node_name, "chosen", 255) == 0) {
    if (strncmp(prop_name, "linux,initrd-start", 255) == 0) {
      initrd_addr = fdt32_to_cpu(*(uint32_t*)value);
    }
  }
}

void set_initrd_end(const char* node_name, const char *prop_name, void* value, uint32_t size) {
  // out target
  // node: chosen prop: linux,initrd-end
  // size 0x00000004
  if (strncmp(node_name, "chosen", 255) == 0) {
    if (strncmp(prop_name, "linux,initrd-end", 255) == 0) {
      initrd_end = fdt32_to_cpu(*((uint32_t*)value));
    }
  }
}

void init_memory() {

  fdt_traverse(set_initrd_addr);
  fdt_traverse(set_initrd_end);
 
  // reserve initrd
  initrd_end = align_addr(initrd_end);
  uint64_t initrd_size = initrd_end - initrd_addr;
  memory_reserve((initrd_addr - MEMORY_BASE) / FRAME_SIZE, initrd_size / FRAME_SIZE);
  kprintf("[K] Initframfs Addr: 0x%lx\n", initrd_addr);
  kprintf("[K] Initframfs Size: 0x%lx\n", initrd_size);

  // reserve spin tables
  memory_reserve(0, 1);
  memory_reserve(0x40000/FRAME_SIZE, 2);

  // reserve kernel img
  uint64_t aligned_kernel_end = align_addr((uint64_t)&__bss_end);
  uint64_t kernel_begin = (uint64_t)&__kernel_begin / FRAME_SIZE * FRAME_SIZE;
  uint64_t kernel_size = aligned_kernel_end - kernel_begin;
  memory_reserve((kernel_begin - MEMORY_BASE) / FRAME_SIZE, kernel_size / FRAME_SIZE);
  kprintf("[K] Kernel Addr: 0x%lx\n", kernel_begin);
  kprintf("[K] Kernel Size: 0x%lx\n", kernel_size);
  
  // reserve device tree
  memory_reserve(fdt_begin / FRAME_SIZE, align_addr(fdt_size) / FRAME_SIZE);
  kprintf("[K] Device Tree Addr: 0x%lx\n", fdt_begin);
  kprintf("[K] Device Tree Size: 0x%lx\n", fdt_size);
  print("[K] Memory Reserved.\n");
}


static void stupidTask() {
  kprintf("<K> Start A Thread in Kernel Space\n");

  uint64_t tmp;
  asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
  tmp |= 1;
  asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));

  const char img[] = "syscall.img";
  struct cpio_newc_header *p_header = find_file(VA_START+initrd_addr, img, strlen(img));
  if (p_header == NULL) {
    kprintf("<K> Failed to find syscall.img\n");
  }

  struct taskControlBlock* currentTask = getCurrentTCB();

  uint32_t filesize = cpio_filesize(p_header);
  int pagecnt = filesize / FRAME_SIZE;
  if (filesize % FRAME_SIZE) pagecnt++;

  uint64_t va_exe = 0x0;
  uint64_t va_stack = 0xffffffffb000;
  
  void* text_page = allocate_user_page(currentTask, va_exe, pagecnt);
  cpio_read(p_header, 0, text_page, filesize); // load exe file
  void* user_stack_page = allocate_user_page(currentTask, va_stack, 4);
  memset(user_stack_page, 0, 4 * FRAME_SIZE);

  currentTask->userStackPage = (void*)va_stack;
  currentTask->userStackExp = 2;
  
  startInEL0((uint64_t)va_exe);
}

int kernel_main() {
  mini_uart_init();
  print("==Sleazy Rpi3b+ Kernel==\n");
  set_exception_vector_table();
  print("[K] Finish setting exception vector table.\n");
  init_frame_allocator();
  init_memory();

  addTask(stupidTask, 10);
  print("[K] Added a stupid task\n");
  startScheduler();
  return 0;
}

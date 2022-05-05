#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "devtree.h"
#include "mini_uart.h"
#include "textio.h"
#include "except.h"
#include "mem.h"
#include "task.h"
#include "timer.h"
#include "interface.h"

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


static void uPrint(char *str) {
  int len = strlen(str);
  uart_write(str, len);
}

static void busy_sleep(int msec) {
  uint64_t t = get_cpu_freq() * msec / 1000;
  while(t--) asm volatile("nop");
}

static void uTest() {
  uPrint("<U> Sleazy Pbi3b+ Kernel test userspace program.\r\n");
  // uart_read(inp, 5);
  // uPrint("[U] Test Uart Read ");
  // uPrint(inp);
  // uPrint("\r\n");

  // asm volatile("b 0");
  exec("syscall.img", NULL);
  
  int counter = 0;
  int pid = fork();
  char buf[100];
  if (pid == 0) {
    while(1) {
      uint64_t spreg;
      asm volatile("mov %0, sp" : "=r"(spreg));
      sprintf(buf, "<Child> Pid = %d, counter = %d, sp = 0x%lx\r\n", getpid(), counter, spreg);
      uPrint(buf);
      counter += 1;
      busy_sleep(1000);
    }
  } else {
    sprintf(buf, "<Father> Pid = %d, ChildPid = %d\r\n", getpid(), pid);
    uPrint(buf);
    exit(0);
  }
  sprintf(buf, "<U> Error!\r\n");
  uart_write(buf, strlen(buf));
}

static void stupidTask() {
  kprintf("<K> Start A Thread in Kernel Space\n");

  uint64_t tmp;
  asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
  tmp |= 1;
  asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
  
  startInEL0((uint64_t)uTest);
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

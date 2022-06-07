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
#include "tmpfs.h"
#include "initramfs.h"
#include "vfs.h"

#define BUF_LEN 256

uint32_t pInitrdAddr;
uint32_t pInitrdEnd;

uint64_t vInitrdAddr;

extern uint64_t __kernel_begin;
extern uint64_t __bss_end;
extern uint64_t fdt_begin;
extern uint64_t fdt_size;


void set_initrd_addr(const char* node_name, const char *prop_name, void* value, uint32_t size) {
  // node: chosen prop: linux,initrd-start
  // size 0x00000004
  if (strncmp(node_name, "chosen", 255) == 0) {
    if (strncmp(prop_name, "linux,initrd-start", 255) == 0) {
      pInitrdAddr = fdt32_to_cpu(*(uint32_t*)value);
    }
  }
}

void set_initrd_end(const char* node_name, const char *prop_name, void* value, uint32_t size) {
  // out target
  // node: chosen prop: linux,initrd-end
  // size 0x00000004
  if (strncmp(node_name, "chosen", 255) == 0) {
    if (strncmp(prop_name, "linux,initrd-end", 255) == 0) {
      pInitrdEnd = fdt32_to_cpu(*((uint32_t*)value));
    }
  }
}

void init_memory() {

  fdt_traverse(set_initrd_addr);
  fdt_traverse(set_initrd_end);
 
  // reserve initrd
  pInitrdEnd = align_addr(pInitrdEnd);
  uint64_t initrdSize = pInitrdEnd - pInitrdAddr;

  vInitrdAddr = pInitrdAddr + VA_START;
  
  memory_reserve(physicalToIndex(pInitrdAddr), initrdSize / FRAME_SIZE);
  kprintf("[K] Initframfs Addr: 0x%lx\n", vInitrdAddr);
  kprintf("[K] Initframfs Size: 0x%x\n", initrdSize);
  // reserve spin tables
  memory_reserve(0, 1);
  /* TODO if use finer granule, this should be modified */
  memory_reserve(physicalToIndex(EL1_PGD), 3);

  // reserve kernel img
  // ??
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

  // reserve framebuffer for vc
  memory_reserve(physicalToIndex(0x3E000000), 0x1000000 / FRAME_SIZE);
  kprintf("[K] Frame Buffer Addr: 0x%lx\n", 0x3E000000);
}

static void stupidTask() {
  kprintf("<StupidTask> Start A Thread in Kernel Space\n");

  /* this is required by syscall.img */
  uint64_t tmp;
  asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
  tmp |= 1;
  asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));

  
  const char img[] = "vfs1.img";
  struct cpio_newc_header *p_header = find_file(vInitrdAddr, img, strlen(img));
  if (p_header == NULL) {
    kprintf("<StupidTask> Failed to find syscall.img\n");
  }

  uint32_t filesize = cpio_filesize(p_header);
  int pagecnt = filesize / FRAME_SIZE;
  if (filesize % FRAME_SIZE) pagecnt++;


  uint64_t va_exe = 0x0;
  uint64_t va_stack = 0xffffffffb000;
  
  void* exe_page = allocate_user_page(currentTask, va_exe, pagecnt);
  cpio_read(p_header, 0, exe_page, filesize);
  
  void* user_stack_page = allocate_user_page(currentTask, va_stack, 4);
  memset(user_stack_page, 0, 4 * FRAME_SIZE);
  
  startInEL0(va_exe, va_stack + 4 * FRAME_SIZE);
}

int kernel_main() {
  mini_uart_init();
  print("==Sleazy Rpi3b+ Kernel==\n");
  set_exception_vector_table();
  print("[K] Finish setting exception vector table.\n");
  init_frame_allocator();
  init_memory();

  if (init_rootfs() < 0) {
    print("[K] Failed to initialized rootfs\n");
  }
  print("[K] Initialized tmpfs as rootfs\n");

  struct vfs *initrdfs;
  if (initrd_create_vfs(&initrdfs, "initrd", vInitrdAddr) < 0) {
    print("[K] Failed to create initrdfs\n");
  }
  if (vfs_register(initrdfs) < 0) {
    print("[K] Failed to register initramfs\n");
  }
  if (vfs_mkdir("/initramfs") < 0) {
    print("[K] Failed to mkdir /initramfs\n");
  }
  initrdfs->vfs_op->vfs_mount(initrdfs, "/initramfs");

  addTask(stupidTask, 10);
  print("[K] Added a stupid task\n");
  startScheduler();
  return 0;
}

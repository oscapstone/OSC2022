#include "mini_uart.h"
#include "string.h"
#include "reboot.h"
#include "cpio.h"
#include "devtree.h"
#include "except.h"
#include "timer.h"
#include "mem.h"
#include <stdio.h>

#define BUF_LEN 1024
#define STR_LEN 256
#define USER_STACK_SIZE 0x2000

#define min(x, y) ((x) < (y) ? (x) : (y))
#define NULL 0

extern void branch_to_address_in_el0(uint64_t instr_addr, uint64_t stack_addr);
extern void set_exception_vector_table();
extern uint64_t MEMORY_LIMIT;
extern uint64_t __kernel_begin;
extern uint64_t __bss_end;
extern uint64_t fdt_begin;
extern uint64_t fdt_size;


uint64_t initrd_addr;
uint64_t initrd_end;

struct cpio_newc_header* find_file(char *target, uint32_t name_len) {
  static char fname[STR_LEN];
  struct cpio_newc_header *p_header = cpio_first(initrd_addr);
  while (p_header != 0) {
    cpio_filename(p_header, fname, STR_LEN);
    if (strncmp(fname, target, min(BUF_LEN, name_len)) == 0) {
      return p_header;
    }
    p_header = cpio_nextfile(p_header);
  }
  return NULL;
}


void print_message(void *data) {
  print("Timeout: ");
  print((char*)(data));
  print_char('\n');
}

void cat() {
  static char input[BUF_LEN];
  memset(input, 0, BUF_LEN);
  print("Filename: ");
  read(input, BUF_LEN);
  struct cpio_newc_header *p_header = find_file(input, BUF_LEN);
  if (p_header != NULL) {
    uint32_t off = 0;
    uint32_t len = 0;
    while ((len = cpio_read(p_header, off, input, BUF_LEN-1)) != 0) {
      input[len] = 0;
      off += len;
      print(input);
    }
    print("\nfilesize: ");
    print_hex(off);
    print(" byte(s)\n");
  } else {
    print("File not found!\n");
  }
}

void set_initrd_addr(const char* node_name, const char *prop_name, void* value, uint32_t size) {
  // out target
  // node: chosen prop: linux,initrd-start
  // size 0x00000004
  if (strncmp(node_name, "chosen", 255) == 0) {
    if (strncmp(prop_name, "linux,initrd-start", 255) == 0) {
      uint32_t initaddr = fdt32_to_cpu(*((uint32_t*)value));
      print_hex(initaddr);
      print_char('\n');
      initrd_addr = initaddr;
    }
  }
}

void set_initrd_size(const char* node_name, const char *prop_name, void* value, uint32_t size) {
  // out target
  // node: chosen prop: linux,initrd-end
  // size 0x00000004
  if (strncmp(node_name, "chosen", 255) == 0) {
    if (strncmp(prop_name, "linux,initrd-end", 255) == 0) {
      uint32_t initaddr = fdt32_to_cpu(*((uint32_t*)value));
      print_hex(initaddr);
      print_char('\n');
      initrd_end = initaddr;
    }
  }
}

void getmemory_size(const char *node_name, const char *prop_name, void *value, uint32_t size) {
  if (strncmp(node_name, "memory@0", 255) == 0) {
    if (strncmp(prop_name, "reg", 255) == 0) {
      print("memory limit: ");
      MEMORY_LIMIT = fdt32_to_cpu(((uint32_t*)value)[1]);
      print_hex(MEMORY_LIMIT);
      print_char('\n');
    }
  }
}

void init_memory() {

  fdt_traverse(set_initrd_addr);
  fdt_traverse(set_initrd_size);
  
  // reserve initrd
  initrd_end = align_addr(initrd_end);
  uint64_t initrd_size = initrd_end - initrd_addr;
  memory_reserve((initrd_addr - MEMORY_BASE) / FRAME_SIZE, initrd_size / FRAME_SIZE);

  // reserve spin tables
  memory_reserve(0, 1);

  // reserve kernel img
  uint64_t aligned_kernel_end = align_addr((uint64_t)&__bss_end);
  uint64_t kernel_begin = (uint64_t)&__kernel_begin / FRAME_SIZE * FRAME_SIZE;
  uint64_t kernel_size = aligned_kernel_end - kernel_begin;
  memory_reserve((kernel_begin - MEMORY_BASE) / FRAME_SIZE, kernel_size / FRAME_SIZE);

  // reserve device tree
  print_hexl(fdt_begin);
  print(" ");
  print_hexl(fdt_size);
  print(" - fdt\n");
  memory_reserve(fdt_begin / FRAME_SIZE, align_addr(fdt_size) / FRAME_SIZE);
}

int kernel_main() {
  static char buf[BUF_LEN];
  set_exception_vector_table();
  mini_uart_init();
  print("Hello Basic Shell!\n");
  fdt_traverse(getmemory_size);
  init_frame_allocator();
  print("finish initializing frame page allocator\n");
  init_memory();
  core_timer_enable();
  uint64_t inf = 0x7fffffffffffffffll;
  asm volatile("msr cntp_cval_el0, %0"
               :
               : "r" (inf));
  while (1) {
    print("# ");
    read(buf, BUF_LEN);
    if (strncmp(buf, "ls", BUF_LEN) == 0) {
      struct cpio_newc_header *p_header = cpio_first(initrd_addr);
      while (p_header != 0) {
        cpio_filename(p_header, buf, BUF_LEN);
        print(buf);
        print_char('\n');
        p_header = cpio_nextfile(p_header);
      }
    } else if (strncmp(buf, "cat", BUF_LEN) == 0) {
      cat();
    } else if (strncmp(buf, "help", BUF_LEN) == 0) {
      print("help     : print this help menu\n");
      print("reboot   : reboot the device\n");
      print("ls       : list all files in the initrd\n");
      print("cat      : print the content of a file\n");
      print("malloc   : test memory management\n");
    } else if (strncmp(buf, "reboot", BUF_LEN) == 0) {
      print("reboot in 100 ticks\n");
      reset(100);
      while(1);
    } else if (strncmp(buf, "malloc", BUF_LEN) == 0) {
      int idx1 = allocate_frame(0);
      print("testing memory management\n");
      print("malloc 48, 16, 76 bytes arrays\n");
      char *ptr1 = kmalloc(48);
      print_hex((uint32_t)ptr1);
      print_char(' ');
      char *ptr2 = kmalloc(16);
      print_hex((uint32_t)ptr2);
      print_char(' ');
      char *ptr3 = kmalloc(76);
      print_hex((uint32_t)ptr3);
      print_char('\n');
      kfree(ptr1);
      kfree(ptr2);
      kfree(ptr3);
      ptr1 = kmalloc(16);
      ptr2 = kmalloc(96);
      ptr3 = kmalloc(108);
      print("\nfree and malloc 108, 96, 16 bytes arrays\n");
      print_hex((uint32_t)ptr1);
      print_char(' ');
      print_hex((uint32_t)ptr2);
      print_char(' ');
      print_hex((uint32_t)ptr3);
      print("\ntest finished\n");
    } else {
      if (buf[0] != '\0') {
        print(buf);
        print(" command not found\n");
      }
    }
  }
  return 0;
}

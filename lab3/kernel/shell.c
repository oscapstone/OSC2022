#include "mini_uart.h"
#include "string.h"
#include "reboot.h"
#include "cpio.h"
#include "devtree.h"
#include "user.h"
#include "except.h"

#define BUF_LEN 1024
#define STR_LEN 256
#define USER_STACK_SIZE 0x2000

#define min(x, y) ((x) < (y) ? (x) : (y))
#define NULL 0

uint64_t initrd_addr;

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
      // print_hex(initaddr);
      initrd_addr = initaddr;
    }
  }
}


void exe() {
  static char input[STR_LEN];
  memset(input, 0, STR_LEN);
  print("Executable Filename: ");
  read(input, STR_LEN);
  struct cpio_newc_header *p_header = find_file(input, STR_LEN);
  if (p_header != NULL) {
    uint32_t off = 0;
    uint32_t len = 0;
    len = cpio_read(p_header, off, input, sizeof(uint32_t));
    if (len < 4) {
      print("filesize is too small cannot read the begin address\n");
      return;
    }
    uint64_t begin_addr = *((uint32_t*)(input));
    print("Begin address: ");
    print_hexl(begin_addr);
    print_char('\n');
    off += len;
    if (begin_addr < 0x120000) {
      print("The begin address should be larger than 0x120000\n");
      return;
    }
    char *target_addr = (char*)begin_addr;
    while ((len = cpio_read(p_header, off, input, STR_LEN)) != 0) {
      memcpy(target_addr, input, len);
      off += len;
      target_addr += len;
    }
    uint64_t stack_addr = (uint64_t)target_addr + USER_STACK_SIZE;
    print("Load the program to the specified address space\n");
    core_timer_enable();
    branch_to_address_in_el0(begin_addr, stack_addr);
  } else {
    print("File not found!\n");
  }
}

void test_async() {
  static char buf[60];
  while (1) {
    uint32_t len = async_read(buf, 64);
    if (strncmp(buf, "stop", len) == 0) break;
    else {
      async_print("recv ");
      async_print(buf);
      async_print("\n");
    }
  }
  while (1) asm volatile("nop");
}

int kernel_main() {
  static char buf[BUF_LEN];
  mini_uart_init();
  print("Hello Basic Shell!\n");
  fdt_traverse(set_initrd_addr);
  set_exception_vector_table();
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
      print("exe      : execute the program in EL0\n");
      print("async    : test async uart io\n");
    } else if (strncmp(buf, "reboot", BUF_LEN) == 0) {
      print("reboot in 100 ticks\n");
      reset(100);
      while(1);
    } else if (strncmp(buf, "exe", BUF_LEN) == 0) {
      exe();
    } else if (strncmp(buf, "async", BUF_LEN) == 0) {
      branch_to_address_in_el0((uint64_t)test_async, 0x150000);
    } else {
      if (buf[0] != '\0') {
        print(buf);
        print(" command not found\n");
      }
    }
  }
  return 0;
}

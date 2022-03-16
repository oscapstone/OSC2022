#include "mini_uart.h"
#include "string.h"
#include "mailbox.h"
#include "reboot.h"
#include "cpio.h"
#include "devtree.h"
#include "mem.h"

#define BUF_LEN 1024

uint64_t initrd_addr;

void cat() {
  static char input[BUF_LEN];
  static char fname[BUF_LEN];
  memset(input, 0, BUF_LEN);
  memset(fname, 0, BUF_LEN);
  print("Filename: ");
  read(input, BUF_LEN);
  struct cpio_newc_header *p_header = cpio_first(initrd_addr);
  while (p_header != 0) {
    cpio_filename(p_header, fname, BUF_LEN);
    if (strncmp(fname, input, BUF_LEN) == 0) {
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
      return;
    }
    p_header = cpio_nextfile(p_header);
  }
  print("File not found!\n");
}

void set_initrd_addr(const char* node_name, const char *prop_name, void* value, uint32_t size) {
  // out target
  // node: chosen prop: linux,initrd-start
  // size 0x00000004

  /*
  print("node: ");
  print(node_name);
  print(" prop: ");
  print(prop_name);
  print_char('\n');
  print_hex(size);
  print_char('\n');
  */
  
  if (strncmp(node_name, "chosen", 255) == 0) {
    if (strncmp(prop_name, "linux,initrd-start", 255) == 0) {
      uint32_t initaddr = fdt32_to_cpu(*((uint32_t*)value));
      // print_hex(initaddr);
      initrd_addr = initaddr;
    }
  }
}

int kernel_main() {
  static char buf[BUF_LEN];
  mini_uart_init();
  print("Hello Basic Shell!\n");
  fdt_traverse(set_initrd_addr);
  while (1) {
    print("# ");
    read(buf, BUF_LEN);
    if (strncmp(buf, "hello", BUF_LEN) == 0) {
      print("Hello World!\n");
    } else if (strncmp(buf, "info", BUF_LEN) == 0) {
      unsigned int revision = get_board_revision();
      unsigned int base_and_size[2] = {0, 0};
      print("revision: ");
      print_hex(revision);
      print_char('\n');
      get_arm_memory(base_and_size);
      print("ARM memory base address: ");
      print_hex(base_and_size[0]);
      print("\nARM memory size: ");
      print_hex(base_and_size[1]);
      print_char('\n');
    } else if (strncmp(buf, "ls", BUF_LEN) == 0) {
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
      print("hello    : print Hello World!\n");
      print("info     : print machine info\n");
      print("reboot   : reboot the device\n");
      print("ls       : list all files in the initrd\n");
      print("cat      : print the content of a file\n");
      print("tmalloc  : test simple_malloc()\n");
    } else if (strncmp(buf, "reboot", BUF_LEN) == 0) {
      print("reboot in 100 ticks\n");
      reset(100);
      while(1);
    } else if (strncmp(buf, "tmalloc", BUF_LEN) == 0) {
      print("test simple_malloc:\n");
      print("s1 = malloc(10); s1 = \"teststr1\"\ns1: ");
      char *s1 = (char*)simple_malloc(10);
      strncpy(s1, "teststr1", 10);
      print(s1);
      print("\ns2 = malloc(5); s2 = \"boom\"\ns2: ");
      char *s2 = (char*)simple_malloc(5);
      strncpy(s2, "boom", 5);
      print(s2);
      print("\ns1: ");
      print(s1);
      print("\ns1 addr: ");
      print_hex((uint32_t)s1);
      print("\ns2 addr: ");
      print_hex((uint32_t)s2);
      print_char('\n');
    } else {
      if (buf[0] != '\0') {
        print(buf);
        print(" command not found\n");
      }
    }
  }
  return 0;
}

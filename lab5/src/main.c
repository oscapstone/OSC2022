#include <stdint.h>
#include <string.h>
#include "devtree.h"
#include "mini_uart.h"
#include "textio.h"
#include "except.h"

#define BUF_LEN 256

uint32_t initrd_addr;

void set_initrd_addr(const char* node_name, const char *prop_name, void* value, uint32_t size) {
  // node: chosen prop: linux,initrd-start
  // size 0x00000004
  if (strncmp(node_name, "chosen", 255) == 0) {
    if (strncmp(prop_name, "linux,initrd-start", 255) == 0) {
      initrd_addr = fdt32_to_cpu(*(uint32_t*)value);
    }
  }
}

int kernel_main() {
  static char buf[BUF_LEN];
  mini_uart_init();
  print("==Sleazy Rpi3b+ Kernel==\n");
  fdt_traverse(set_initrd_addr);
  kprintf("[K] Initframfs Addr: 0x%lx\n", initrd_addr);
  set_exception_vector_table();
  print("[K] Finish setting exception vector table.\n");
  return 0;
}

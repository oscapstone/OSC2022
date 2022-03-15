#include "mini_uart.h"
#include "shell.h"
#include "initrd.h"
#include "dtb.h"
#include "utils.h"

extern char* initrd_addr;
extern void *_dtb_addr;

static void get_initrd(int type, const char *name, const void *data, uint32_t size) {
  if (type == FDT_PROP && !compare_string(name, "linux,initrd-start")) {
    initrd_addr = (char *)(uintptr_t)get_be_int(data);
  }
}

void kernel_main(void)
{	
	traverse_device_tree(_dtb_addr, get_initrd);
	uart_init();
	shell();
}
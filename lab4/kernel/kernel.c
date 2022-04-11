#include "mini_uart.h"
#include "shell.h"
#include "devtree.h"
#include "cpio.h"
#include "mm.h"

void kernel_main(void)
{
	uart_init();
	init_mm();
	devtree_getaddr();
	fdt_traverse(initramfs_callback);
	uart_send_string("OSDI 2022 Spring\n");

	shell_loop();
}
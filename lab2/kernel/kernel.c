#include "mini_uart.h"
#include "shell.h"
#include "devtree.h"
#include "cpio.h"

void kernel_main(void)
{
	uart_init();
	devtree_getaddr();
	fdt_traverse(initramfs_callback);
	uart_send_string("OSDI 2022 Spring\n");

	shell_loop();
}
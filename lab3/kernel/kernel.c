#include "mini_uart.h"
#include "shell.h"
#include "devtree.h"
#include "cpio.h"
#include "exception.h"

void kernel_main(void)
{
	uart_init();
	devtree_getaddr();
	fdt_traverse(initramfs_callback);
	enable_interrupt();
	uart_send_string("OSDI 2022 Spring\n");

	shell_loop();
}
#include "mini_uart.h"
#include "shell.h"
#include "exception.h"

void kernel_main(void)
{
	uart_init();
	enable_interrupt();
	shell();
}
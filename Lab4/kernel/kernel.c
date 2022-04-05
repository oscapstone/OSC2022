#include "mini_uart.h"
#include "shell.h"
#include "exception.h"
#include "timer.h"
#include "memory.h"


void kernel_main(void) {
	uart_init();
	memory_init();
	enable_interrupt();
	shell();
}
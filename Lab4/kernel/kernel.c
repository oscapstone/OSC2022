#include "mini_uart.h"
#include "shell.h"
#include "exception.h"
#include "timer.h"
#include "memory.h"
#include "allocator.h"


void kernel_main(void) {
	uart_init();
	memory_init();
	init_reserve();
	enable_interrupt();
	shell();
}
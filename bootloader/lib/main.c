#include "uart.h"
#include "shell.h"
#include "my_string.h"
#include "utils.h"


void main()
{
	register unsigned long x0 asm("x0");
    extern unsigned long DTB_BASE;
	DTB_BASE = x0;

	uart_init();
	boot_msg();

	char cmd[MAX_LEN];
	cmd_input(cmd);
}

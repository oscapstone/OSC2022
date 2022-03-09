#include "uart.h"
#include "shell.h"
#include "my_string.h"


void main()
{
	uart_init();
	boot_msg();

	char cmd[MAX_LEN];
	cmd_input(cmd);
}

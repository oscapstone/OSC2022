#include "uart.h"
#include "my_string.h"
#include "command.h"
#include "uart_bootloader.h"
#include "cpio.h"
#include "allocator.h"

void shell_get_command(char *cmd, int lim);
void shell_execute(char *cmd);

void shell_get_command(char *cmd, int lim)
{
	int c;
	while (--lim > 0 && (c = uart_getc()) != '\n') {
		uart_send(c);
		*cmd++ = c;
	}
	*cmd = '\0';
	
	// print \n\r
	uart_printf("\0");
}

void shell_execute(char *cmd)
{
	if (!str_cmp(cmd, "help")) {
		uart_printf("help:\t print this help menu");
		uart_printf("hello:\t print Hello World!");
		uart_printf("reboot:\t reboot the device");
		uart_printf("loadimg:\t bootloader self relocation first, then load img by host2pi.py");
		uart_printf("ls:\t list all file (parse the .cpio)");
		uart_printf("cat:\t print file content (parse the .cpio)");
		uart_printf("alloc:\t test the function simple_alloc()");
	}
	else if (!str_cmp(cmd, "hello")) {
	    uart_printf("I'm a kernel8 with slef-relocatted bootloader 8888");
	}
	else if (!str_cmp(cmd, "reboot")) {
	    uart_printf("rebooting...");
	    reset(150);
	}
	else if (!str_cmp(cmd, "loadimg")) {
		loadimg();	
	}
	else if (!str_cmp(cmd, "ls")) {
		cpio_list();	
	}
	else if (!str_cmp(cmd, "cat")) {
		uart_printf("Input file name:");
		int max_file_name = 100;
		char s[max_file_name];
		uart_get_string(s, max_file_name);
		uart_printf("\r\n%s:", s);
		cpio_cat(s);	
	}
	else if (!str_cmp(cmd, "alloc")) {
	    char *s = simple_alloc(10);
	    if (*s) {
    		s = "abcdefefg\0";
    		uart_printf("%s", s);
	    }
    	else {
			uart_printf("no space in heap");
		}
	}
	else {
		uart_printf("ERROR: unsupport shell command : %s", cmd);
	}
}

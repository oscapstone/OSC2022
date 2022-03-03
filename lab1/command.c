#include "uart.h"
#include "string.h"
#include "mailbox.h"
#include "reboot.h"

void command_help ()
{
    uart_puts("help:\tprint this help menu\n");
    uart_puts("hello:\tprint Hello World!\n");
    uart_puts("reboot:\treboot the device\n");
    uart_puts("\n");
}

void command_hello ()
{
    uart_puts("Hello World!\n");
}


void command_notfound (char * s) 
{
    uart_puts("Err: command ");
    uart_puts(s);
    uart_puts(" not found, try <help>\n");
}

void command_mailbox(){
    get_board_revision();
    get_arm_memory();
}

void command_reboot()
{
    uart_puts("Start Rebooting...\n");

    reset(100);
    
	while(1);
}
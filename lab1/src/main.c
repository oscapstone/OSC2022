#include "uart.h"
#include "shell.h"
#include "mailbox.h"

unsigned long __stack_chk_guard;
void __stack_chk_guard_setup(void)
{
     __stack_chk_guard = 0xBAAAAAAD;//provide some magic numbers
}

void __stack_chk_fail(void)                         
{
 /* Error message */                                 
}

#define MAXCMD 30	// max length of command

void main()
{
    // set uart
    uart_init();
    uart_printf("uart init");
    
    // print raspi revision
    mailbox_get_board_revision();
    // print memory base addr and size
    mailbox_get_arm_memory();
    
    // echo everything back
    while(1) {
    	char cmd[MAXCMD];
        shell_get_command(cmd, MAXCMD);
        shell_execute(cmd);
    }
}

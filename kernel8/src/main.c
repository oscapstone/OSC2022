#include "uart.h"
#include "shell.h"
#include "mailbox.h"
#include "allocator.h"

/*
unsigned long __stack_chk_guard;
void __stack_chk_guard_setup(void)
{
     __stack_chk_guard = 0xBAAAAAAD;//provide some magic numbers
}

void __stack_chk_fail(void)                         
{                              
}
*/

#define MAXCMD 30	// max length of command

void main()
{
    // set uart
    uart_init();
    uart_flush();
    char *s = simple_alloc(10);
    s = "ya\0";
    uart_printf("%s", s);
    uart_printf("uart init");

    // print raspi revision
    mailbox_get_board_revision();
    // print memory base addr and size
    mailbox_get_arm_memory();
    
    char cmd[MAXCMD]; 
    // echo everything back
    while(1) {  
        shell_get_command(cmd, MAXCMD);
        shell_execute(cmd);
    }
}

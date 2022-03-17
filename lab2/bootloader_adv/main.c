#include "uart.h"
#include "shell.h"
#include "mailbox.h"

void main()
{
    // set up serial console
    uart_init();
    
    // say hello
    get_board_revision();
    get_arm_memory();
    
    // echo everything back
    shell_start();
}

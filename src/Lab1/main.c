#include "mini_uart.h"
#include "shell.h"

void main()
{
    // set up serial console
    uart_init();
    // execute shell
    exe_shell();
    
    return 0;
}
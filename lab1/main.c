#include "uart.h"
#include "shell.h"

#define CMD_LEN 128

enum shell_status {
    Read,
    Parse
};

void main()
{
    // set up serial console
    uart_init();
    
    // say hello
    uart_puts("UART Ready\n");
    
    /*
    // echo everything back
    while(1) {
        uart_send(uart_getc());
    }
    */

    shell_init();
    enum shell_status status = Read;

    while (1) {
        char cmd[CMD_LEN];
        switch (status) {
            case Read:
                shell_input(cmd);
                status = Parse;
                break;

            case Parse:
                shell_controller(cmd);
                status = Read;
                break;
        }
    }
    
}

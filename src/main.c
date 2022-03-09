#include "uart.h"
#include "shell.h"
#include "command.h"

int main(){
    // set up serial console
    uart_init();
    // print hello message
    print_welcome();
    // start shell
    shell_start();

    return 0;
}
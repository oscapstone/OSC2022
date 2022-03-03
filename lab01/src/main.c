#include "uart.h"
#include "shell.h"
#include "mbox.h"

void main(){
    // set up serial console
    uart_init();

    get_board_revision();
    get_arm_memory();
    
    // uart_puts
    uart_puts("rpi3 is ready\n");

    // start shell
    shell_start();

}

#include "uart.h"
#include "shell.h"

void kernel_main(){
    // set up serial console
    // dtb_init(x0);
    uart_init();
    // uart_hex(x0);
    // start shell
    shell();
}

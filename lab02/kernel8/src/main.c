#include "uart.h"
#include "shell.h"
#include "dtb.h"
void kernel_main(){
    // set up serial console
    uart_init();

    dtb_init();
    shell();
}

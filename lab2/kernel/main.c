// #include "shell.h"
#include "stdint.h"
// #include "uart.h"

void kernel_main() {
    // initialize UART for Raspi2
    // uart_init();
    uart_write_string("good\r\n");

    // shell();
    // load_kernel();
}
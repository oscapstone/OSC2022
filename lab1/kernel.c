#include "mini_uart.h"
#include "shell.h"

void kernel_main(void)
{
    uart_init();
    uart_send_string("Hello, world!\r\n");
    shell();

    // uart_init();
    // uart_send_string("Hello, world!\r\n");

    // while (1) {
    //     uart_send(uart_recv());
    //     uart_send_string("~~~~~!\r\n");
    // }
}
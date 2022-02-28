#include "uart.h"
#include "shell.h"

int main()
{
    uart_init();
    char input[20] = "";

    uart_puts("Welcome!!!\n");

    while(1) {
        shell(input);
    }
    return 0;
}

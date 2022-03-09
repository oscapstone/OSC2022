#include "uart.h"

unsigned int addr;

void main() {
    uart_init();

    volatile unsigned char *prog = (unsigned char *)0x90000;
    unsigned char code;
    unsigned int size = 0;

    uart_puts("Waiting for a program to be sent from the UART ...\n");

    uart_puts("Start get code\n");

    for (size = 0; size < 2616; size++) {
        code = uart_getc();
        prog[size] = code;
        uart_hex(size);
        uart_puts("\n");
    }

    uart_puts("\n");

    uart_puts("Executes code at 0x");
    uart_hex(addr);
    uart_puts(", size = 0x");
    uart_hex(size);
    uart_puts("\n----------------------------------\n");

    asm volatile("mov x1, #0x90000");
    asm volatile("br x1");
}

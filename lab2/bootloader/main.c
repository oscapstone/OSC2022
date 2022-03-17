#include "uart.h"

void main() {
    uart_init();

    volatile unsigned char *prog = (unsigned char *)0x80000;
    unsigned char code;
    unsigned int size = 0;

    while ((code = uart_getc()) != '\n')
        ;

    volatile unsigned char *bootloader = (unsigned char *)0x60000;
    volatile unsigned char *cur = (unsigned char *)0x80000;
    for (int i = 0; i < 0x2000; i++) {
        *(bootloader + i) = *(cur + i);
    }

    asm volatile("b #-0x1FFFC");

    uart_puts("Waiting for a kernel to be sent from the UART ...\n");

    uart_puts("Start get kernel ...\n");

    for (size = 0; size < 2508; size++) {
        code = uart_getc_raw();
        prog[size] = code;
    }

    uart_puts("\n");

    uart_puts("Execute Kernel\n");

    asm volatile("mov x1, #0x80000");
    asm volatile("br x1");
}

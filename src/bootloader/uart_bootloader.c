#include "mini_uart.h"

void mini_uart_boot()
{
    unsigned int size=0;
    char *kernel=(char*)0x80000;
    volatile unsigned long dtb_base;
    asm volatile ("mov %0, x0" : "=r" (dtb_base));
    // set up serial console
    uart_init();
    // int i = 10000;
    // while(i) i--;
    // uart_puts("waiting for kernel image...\n");

    // read the kernel's size
    for (int i = 0; i < 4; i++) {
        char b = uart_getc();
        size = size << 8;
        size = size | b;
    }

    /*uart_puts("recieving kernel image...\n");
    uart_puts("file size: 0x");
    uart_hex(size);
    uart_puts(" bytes\n");*/

    // read the kernel
    for (int i = 0; i < size; i++) {
        kernel[i] = uart_getc();
    }
    // uart_puts("reading finish");

    // restore arguments and jump to the new kernel.
    asm volatile ("mov x0, %0" : : "r" (dtb_base));
    asm volatile (
        // we must force an absolute address to branch to
        // x30 use as link register when returning from subroutine
        "mov x30, 0x80000; ret"
    );
}
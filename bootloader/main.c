#include "uart.h"

void main()
{
    int size=0;
    char *kernel=(char*)0x80000;

    // set up serial console
    uart_init();

    while(1) {
        if (uart_getc() == '\n')
            break;
        else {
            uart_send('p');
            uart_send('a');
            uart_send('s');
            uart_send('s');
            uart_send('w');
            uart_send('o');
            uart_send('r');
            uart_send('d');
            uart_send(':');
            uart_send('\r');
            uart_send('\n');
        }
    }

    // read the kernel's size
    for (int i=0; i<7; i++) {
        size = size*10 + uart_getc() - '0';
    }

    // read the kernel
    while(size--) {
        *kernel++ = uart_getc();
    }

    // jump to the new kernel.
    asm volatile (
        // we must force an absolute address to branch to
        "mov x0, x10;"
        "mov x1, x11;"
        "mov x2, x12;"
        "mov x3, x13;"
        "mov x30, 0x80000;"
        "ret"
    );
}

#include "uart.h"
#include "lib.h"

// int isRelocated = 0;

void main(void *dtb)
{
    char *s;
    char *d;
    char header[BUFFER_SIZE];
    int i, length = 0;

    // if (isRelocated == 0) {
    //     isRelocated = 1;

    /* relocate bootloader */
    s = (char *) 0x80000;
    d = (char *) 0x60000;
    for (i = 0;i<0x1f000;i++) {
        *d = *s;
        s++;
        d++;
    }
    asm volatile(
        "adr    x9,     .\n"
        "sub    x9,     x9,     0x20000\n"
        "add    x9,     x9,     0x10\n"
        "br     x9\n"
    );

    uart_init();
    uart_puts("\033[2J\033[H");
    uart_hex(dtb);
    uart_puts("\n");
    uart_puts("Simple Bootloader\n");
    uart_puts("version: 0.7.0.alpha\n\n");
    uart_puts("Loading kernel image from mini UART ...\n");
    uart_puts("img size :");
    
    for (i=0; i<BUFFER_SIZE; i++) {
        header[i] = '\0';
    }
    
    /* get img size */
    uart_gets(header);
    length = atoi(header);
    uart_int(length);
    uart_puts("\n");

    /* load img into memory starting at address 0x80000 */
    s = (char *) 0x80000;
    for (i = 0;i<length;i++) {
        *s = uart_getc(ECHO_OFF);
        uart_int(i);
        uart_puts(": ");
        uart_hex(*s);
        uart_puts("\n");
        s++;
    }
    // }

    /* jump to address 0x80000 */
    asm volatile(
        "mov    x9,     0x0000                  \n"
        "movk   x9,     0x0008,     lsl     16  \n"
        "BR     x9                              \n"
    );
}
#include "uart.h"

void loadimg() {
    long long address = 0x80000;

    unsigned long DTB_BASE;
    register unsigned long x20 asm("x20");
	DTB_BASE = x20;
    printf("%x\r\n",DTB_BASE);
    printf("123\r\n");

    uart_puts("Send image via UART now!\n");

    // big endian
    int img_size = 0, i;
    for (i = 0; i < 4; i++) {
        img_size <<= 8;
        img_size |= (int)uart_getc_raw();
    }

    char *kernel = (char *)address;

    for (i = 0; i < img_size; i++) {
        char b = uart_getc_raw();
        printf("%d: %x\n",i,b);
        *(kernel + i) = b;
    }

    uart_puts("done\n");
    void (*start_os)(void) = (void *)kernel;
    start_os();
}
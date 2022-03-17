#include "stdint.h"

extern char _kernel_start[];  // 0x80000
char* _dtb;

void load_kernel() {
    uint32_t size;
    uart_read(&size, 4);            // read 4 byte magic code
    uart_write_string("[+] Loading kernel8.img from UART\r\n");
    uart_read(&size, 4);            // read kernel8.img size
    uart_write_string("[+] Get kernel8.img size: 0x");
    uart_puth(size);
    uart_write_string("\r\n");
    uart_read(_kernel_start, size); // read kernel8.img to 0x80000
    uart_write_string("[+] Loading kernel8.img successfully!\r\n");
}

void bootloader_main(char* x0) {
    _dtb = x0;      // store x0
    uart_init();
    load_kernel();
    ((void (*)(char*))_kernel_start)(_dtb);
}
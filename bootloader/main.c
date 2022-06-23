#include "uart.h"

extern char _kernel_start[];  // 0x80000
char* _dtb;

// little endian to unsigned int (4 bytes)
unsigned int LE_to_uint(char *s) {
    unsigned int res = 0;
    res += (unsigned int)(s[0] & 0xff);
    res += (unsigned int)((s[1] & 0xff) << 8);
    res += (unsigned int)((s[2] & 0xff) << 16);
    res += (unsigned int)((s[3] & 0xff) << 24);
    return res;
}

void load_kernel() {
    char size_buf[5] = {0};
    unsigned int size = 0;
    uart_read(size_buf, 4);         // read kernel8.img size
    size = LE_to_uint(size_buf);
    uart_write_string("[+] Get kernel8.img size : 0x");
    uart_write_hex(size);
    uart_write_string("\r\n");
    uart_read(_kernel_start, size); // read kernel8.img to 0x80000
    uart_write_string("[+] Successfully load the kernel8.img\r\n");
}

void bootloader_main(char* x0) {
    _dtb = x0;      // store x0
    uart_init();
    load_kernel();
    ((void (*)(char*))_kernel_start)(_dtb);
}
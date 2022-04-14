extern char _kernel_start[];  // 0x80000
char* _dtb;

void load_kernel() {
    unsigned int size = 0;
    uart_read(&size, 4);            // read kernel8.img size
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
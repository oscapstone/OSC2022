#include "uart.h"
#include "shell.h"
#include "fdt.h"

void main()
{
    register unsigned long dtb asm("x18");
    uart_init();
    uart_puts("\n");
    uart_puts("\n");
    uart_hex(dtb);
    uart_puts("\n");
    traverse_device_tree(dtb,dtb_callback_initramfs);
    shell();
}

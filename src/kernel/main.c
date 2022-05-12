#include "uart.h"
#include "shell.h"
#include "dtb.h"

void main()
{
    register unsigned long x0 asm("x0");
    unsigned long DTB_BASE = x0;

    // set up serial console
    uart_init();

    asyn_uart_init();

    fdt_parser((fdt_header *)(DTB_BASE), initramfs_callback);

    uart_puts("\n\r");

    memory_init();

    thread_init();
    // execute shell
    exe_shell();
    
    return;
}
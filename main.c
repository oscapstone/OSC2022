#include "uart.h"
#include "devicetree.h"
#include "shell.h"
#include "mem.h"
#include "timer.h"
void main() {
    // set up serial console
    uart_init();

    // set up initramfs address
    if (fdt_traverse(cpio_callback))
            printf("error no cpio\n");
    // say hello
    // uart_puts("Hello World!\n");
    
    // init timer
    timer_init();

    // init memory
    init_buddy();
    
    // start shell
    shell();
}

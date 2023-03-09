#include "init/kernel_init.h"

extern int __heap_start;
void kernel_init(void *dtb){
    init_core_timer();
    mini_uart_init();
    mini_uart_irq_init();

    local_irq_enable();
    INFO("kernel start initialization...");

    // memory management initialization
    mm_init(dtb); 

    fdt_parser(dtb, fdt_initrdfs_callback); 
    return; 
}


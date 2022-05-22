#include "init/kernel_init.h"

extern int __heap_start;
void kernel_init(void *dtb){
    init_core_timer();
    mini_uart_init();
    mini_uart_irq_init();
    INFO("kernel start initialization...");

    // init_malloc_state should be executed before simple_malloc is using
    init_malloc_state(&__heap_start);
    fdt_parser(dtb, fdt_initrdfs_callback); 
    mm_init(dtb); 
    return; 
}


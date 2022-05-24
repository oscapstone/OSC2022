#include "init/kernel_init.h"

extern int __heap_start;
void kernel_init(void *dtb){
	local_irq_disable();		
    init_core_timer();
    mini_uart_init();
    mini_uart_irq_init();
	local_irq_enable();		

    INFO("kernel start initialization...");

    // init_malloc_state should be executed before simple_malloc is using
    init_malloc_state(&__heap_start);
    mm_init(dtb); 
    fdt_parser(dtb, fdt_initrdfs_callback); 
    return; 
}


#include "types.h"
#include "peripherals/mini_uart.h"
#include "fs/initrdfs.h"
#include "utils.h"
#include "kernel/timer.h"
extern int __heap_start;
void kernel_init(void *dtb){
    init_core_timer();
    INFO("kernel start initialization...");
    // init_malloc_state should done right after init_core_timer!!!
    init_malloc_state(&__heap_start);
    fdt_parser(dtb, fdt_initrdfs_callback); 
//    initrdfs_init(&__initrdfs_start);
    return; 
}


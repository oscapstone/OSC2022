#include "types.h"
#include "peripherals/mini_uart.h"
#include "fs/initrdfs.h"
#include "utils.h"
extern int __heap_start;
extern int __initrdfs_start;
void kernel_init(void){
    init_malloc_state(&__heap_start);
    initrdfs_init(&__initrdfs_start);
    return; 
}


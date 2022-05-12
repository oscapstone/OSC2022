#include "types.h"
#include "peripherals/mini_uart.h"
#include "init/kernel_init.h"
#include "utils.h"
#include "debug/debug.h"
#include "kernel/shell.h"

void kernel_main(void){
    kernel_init();
    DEBUG_KERNEL_START();

    simple_shell();
}

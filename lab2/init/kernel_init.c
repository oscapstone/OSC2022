#include "types.h"
#include "peripherals/mini_uart.h"

void kernel_init(void){
    mini_uart_init();
    return; 
}

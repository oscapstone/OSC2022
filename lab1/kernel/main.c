#include "types.h"
#include "peripherals/mini_uart.h"
#include "init/kernel_init.h"

void kernel_main(void){
    kernel_init();
    
    write_str("Hello world!\r\n");
    while(1) mini_uart_write(mini_uart_read());
}

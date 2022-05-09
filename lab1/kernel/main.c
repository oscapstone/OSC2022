#include "types.h"
#include "peripherals/mini_uart.h"
#include "init/kernel_init.h"
#include "utils.h"
char * hello = "hello world";

void kernel_main(void){
    kernel_init();
    printf("%s %p %x\n", hello, hello, hello);
    printf("%p\n", mini_uart_read);
    while(1){ 
        //register uint8_t tmp = mini_uart_read();
       // write_hex(mini_uart_read());
       // mini_uart_write(' ');
        
    };
}

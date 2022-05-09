#include "types.h"
#include "peripherals/mini_uart.h"
#include "init/kernel_init.h"
#include "utils.h"
#include "debug/debug.h"
char * hello = "hello world\n";

int tt[212];
void kernel_main(void){
    int count = 0;
    kernel_init();
    DEBUG_KERNEL_START();
    while(1){ 
        LOG("%d: %c\r\n",count, mini_uart_read());
    };
}

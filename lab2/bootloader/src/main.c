#include "uart.h"
#include "relocate.h"
#include "loadimg.h"

int rel = 1;

void main(){

    if(rel){
        asm("mov x20, x19");
        rel = 0;
        relocate();
    }
    
    uart_init();

    uart_puts("Waiting for loading kernel...\r\n");

    loadimg();

}
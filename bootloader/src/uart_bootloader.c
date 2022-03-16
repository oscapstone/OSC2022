#include "uart_bootloader.h"
#include "uart.h"

void load_new_kernel();

void loadimg(){
	load_new_kernel();
}

void load_new_kernel(){   
    // get new img size through host2pi.py
    uart_printf("Please kill the screen(ctrl+a, k, y)");
    uart_printf("Please execute host2pi.py to receive new kernel size, and pi will receive new kernel through uart");
    int size = uart_get_int();

    char *current_addr = KERNEL_ADDR;
	char c;
    for(int i = 0; i < size; ++i){
        c = uart_getc_raw();
        *current_addr++ = c;
    }
    
    // execute start.S
    void (*new_kernel_start)(void) = (void*)KERNEL_ADDR;
    new_kernel_start();
}

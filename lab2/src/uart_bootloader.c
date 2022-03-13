#include "uart_bootloader.h"
#include "uart.h"

void load_new_kernel();

extern char __start[];		// define in linker.ld?
extern char __end[];

void loadimg(){
	char *cur_addr = __start, *end_addr = __end, *target_addr = TEMP_ADDR;
	
	uart_printf("Copy bootloader to 0x60000");
	while (cur_addr <= end_addr) {
		*target_addr++ = *cur_addr++;
	}
	uart_printf("Copy bootloader finish");
	
	// function move to new addr, so have to call by function pointer ?
    void (*func_ptr)() = load_new_kernel;
    unsigned long int func_addr = (unsigned long int)func_ptr;
    void (*function_call)(void) = (void (*)(void))(func_addr - (unsigned long int)__start + TEMP_ADDR);
    function_call();
}

void load_new_kernel(){   
    // input new img size (know the size through host2pi.py)
    int size = uart_get_int();
    uart_printf("New kernel size: %d", size);

    uart_printf("Copy new kernel to 0x80000");
    char *current_addr = KERNEL_ADDR;
	char c;
    for(int i = 0; i < size; i++){
        c = uart_getc();
        *current_addr++ = c;
    }
    uart_printf("Transmit done.");
    
    // move pc ?
    void (*new_kernel_start)(void) = (void*)KERNEL_ADDR;
    new_kernel_start();
}

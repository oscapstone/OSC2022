
#include "uart.h"
#include "uart_boot.h"
extern char __start[];
extern char __end[];

void copy_new_kernel(){
    char* new_kernel_addr = ((char*)0x80000);
    uart_puts("Start copying new kernel to 0x80000\n");
    uart_puts("kernel_size:");
    int kernel_size = uart_get_int();
    uart_puts("\n");
    for(int i=0;i<kernel_size;i++){
        char c = uart_getc();
        uart_send(c);
        *(new_kernel_addr+i) = c;
    }
    void (*new_kernel_start)(void) = (void*)new_kernel_addr;
    new_kernel_start();
}

void kernel_load(){
    uart_puts("Start copying old kernel to 0x60000\n");
    char* old_kernel_addr = ((char*)0x60000);
    for(char* current_addr = __start;current_addr<=__end;current_addr++,old_kernel_addr++){
        *old_kernel_addr = *current_addr;
    }
    void (*old_kernel_func)(void) = (void (*)(void))((unsigned long int)copy_new_kernel - (unsigned long int)__start + (unsigned long int)0x60000);
    old_kernel_func();
}
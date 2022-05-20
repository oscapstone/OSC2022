#include "types.h"
#include "peripherals/mini_uart.h"
#include "lib/print.h"

void recv_boot_header(){
    while('A' != mini_uart_read());  
}

size_t recv_kernel_base(){
    uint8_t size[8];
    for(uint64_t i = 0 ; i < 8 ; i++) size[i] = mini_uart_read();
    return *(size_t*)size;
}

size_t recv_kernel_size(){
    uint8_t size[8];
    for(uint64_t i = 0 ; i < 8 ; i++) size[i] = mini_uart_read();
    return *(size_t*)size;
}

void recv_kernel(uint64_t base, size_t size){
    uint8_t *pb;
    size_t i;
    for(pb = (uint8_t*)base , i = 0 ; i < size ; i++){
        pb[i] = mini_uart_read();
    } 
}
uint64_t uart_recv_kernel(uint64_t dtb){
    size_t size;
    uint64_t base;
    mini_uart_init();
    
    recv_boot_header();
//    printf("Receive A...\r\n");

    base = recv_kernel_base();
//    printf("Receive kernel base 0x%x...\r\n", base);

    size = recv_kernel_size();
//    printf("Receive kernel size 0x%x...\r\n", size);

    recv_kernel(base, size);  
//    printf("Jump to kernel iamge...\r\n");

    delay_cycles(10000);
    ((void (*)(uint64_t))base)(dtb);
}

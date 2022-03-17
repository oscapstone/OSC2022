#include "bootload.h"
#include "mini_uart.h"
extern unsigned int _start;
extern unsigned int _end;
extern unsigned int _relocate_addr;
extern unsigned int _boot_size;

void relocate(unsigned int* rel_addr){
    char *bootloader = (char*)0x60000;
    char *start = (char*)&_start;


    for(int i=0;i<(unsigned long)(&_boot_size);i++){ 
        bootloader[i] = start[i];
    }

    void (*foo)(void) = (void*)bootloader;
    foo();

}

void bootload_image(unsigned long dtb_addr){
    char *kernel_l = (char *)(0x80000);

    int kernel_size = read_int();
    
    writes_uart("Received file size:");
    writehex_uart((unsigned int)kernel_size);
    writes_uart("\r\n");
    
    for(int i=0;i<kernel_size;i++){
        kernel_l[i] = read_uart();
    }
    
    void (*foo2)(unsigned long) = (void*)kernel_l;
    foo2(dtb_addr);
}
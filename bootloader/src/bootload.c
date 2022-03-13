#include "bootload.h"
#include "mini_uart.h"
extern unsigned int _start;
extern unsigned int _end;
extern unsigned int _relocate_addr;
extern unsigned int _boot_size;
extern void branch_to_address( void * );

void relocate(unsigned int* rel_addr){
    //char *bootloader = (char*)rel_addr;
    char *bootloader = (char*)0x60000;
    //writehex_uart((unsigned int)bootloader);
    char *start = (char*)&_start;

    //void (*foo)(char*) = (void (*)())(char*)0x60000;
    
    //unsigned int size = (unsigned int)&_boot_size;
    //writehex_uart((unsigned int)size);
    //writes_uart("\r\n");

    for(int i=0;i<&_boot_size;i++){ 
        //bootloader[i] = ((char*)&_start)[i];
        bootloader[i] = start[i];
    }
    //foo();
    //_start = 0x60000;
    // asm volatile (
    //      "mov x10, 0x60000;"
    //      "br x10;"
    // );
    void (*foo)(void) = (void*)bootloader;
    foo();
    //branch_to_address((void*)0x60000);

}

void bootload_image(){
    //char *kernel = (char *)(&_start);
    char *kernel_l = (char *)(0x80000);
    //void (*foo)(void) = (void (*)())kernel_addr;
    
    int kernel_size = read_int();

    writes_uart("Received file size:");
    writehex_uart((unsigned int)kernel_size);
    writes_uart("\r\n");
    
    for(int i=0;i<kernel_size;i++){
        //char c = read_uart();
        //writehex_uart((unsigned int)i);
        //writes_uart("\r\n");
        kernel_l[i] = read_uart();
    }
    //writes_uart("Kernel image receive done.\r\n");
    // asm volatile (
    //     "mov x0, x10;"
    //     "mov x1, x11;"
    //     "mov x2, x12;"
    //     "mov x3, x13;"
    //      "mov x10, 0x80000;"
    //      "br x10;"
    // );
    
    void (*foo2)(void) = (void*)kernel_l;
    foo2();
}
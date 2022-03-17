#include "mini_uart.h"
#include "cpio.h"
#include "shell.h"
#include "stdlib.h"
#include "dtb.h"
#include "string.h"
extern unsigned long _head_start_brk;
uint32_t* cpio_addr;
void initramfs_callback(fdt_prop* prop,char * name,uint32_t len_prop){
    if(strncmp(name,"linux,initrd-start",18)==0){ // start address of initrd
        writes_uart("Found target property: \"");
        writes_n_uart(name,18);
        writes_uart("\" at address ");
        cpio_addr = (unsigned int*)((unsigned long)big2little(*((uint32_t*)(prop+1))));
        writehex_uart((unsigned long)cpio_addr);
        writes_uart("\r\n");
        //writehex_uart(len_prop);
        //writehex_uart(*((unsigned int*)(prop+sizeof(prop))));
    }
}

int main(){
    // register unsigned long dtb_reg asm ("x15");

    init_uart();
    writes_uart("\r\n");
    writes_uart("██╗    ██╗███████╗██╗      ██████╗ ██████╗ ███╗   ███╗███████╗\r\n");
    writes_uart("██║    ██║██╔════╝██║     ██╔════╝██╔═══██╗████╗ ████║██╔════╝\r\n");
    writes_uart("██║ █╗ ██║█████╗  ██║     ██║     ██║   ██║██╔████╔██║█████╗  \r\n");
    writes_uart("██║███╗██║██╔══╝  ██║     ██║     ██║   ██║██║╚██╔╝██║██╔══╝  \r\n");
    writes_uart("╚███╔███╔╝███████╗███████╗╚██████╗╚██████╔╝██║ ╚═╝ ██║███████╗\r\n");
    writes_uart("╚══╝╚══╝ ╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝\r\n");

    
    // writes_uart("Loaded dtb address: ");
    // writehex_uart(dtb_reg);
    // writes_uart("\r\n");
    
    fdt_traverse(initramfs_callback);

    writes_uart("Malloc string at address: ");
    writehex_uart((unsigned long)&_head_start_brk);
    writes_uart("\r\n");
    char* string = simple_malloc(8);
    string[0]=1;
    string[1]=2;
    string[2]=3;
    string[3]=4;
    string[4]=5;                                            
    while(1){
        
        read_command();
    }
    return 0;
}
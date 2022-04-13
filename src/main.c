#include "mini_uart.h"
#include "exception.h"
#include "cpio.h"
#include "shell.h"
#include "stdlib.h"
#include "dtb.h"
#include "string.h"
#include "timer.h"
#include "task.h"
#include "allocator.h"
extern unsigned long _head_start_brk;

uint32_t* cpio_addr;
void initramfs_callback(fdt_prop* prop,char * name,uint32_t len_prop){
    if(strncmp(name,"linux,initrd-start",18)==0){ // start address of initrd
        writes_uart("Found target property: \"");
        writes_n_uart(name,18);
        writes_uart("\" at address ");
        cpio_addr = (unsigned int*)((unsigned long)big2little(*((uint32_t*)(prop+1))));
        writehex_uart((unsigned long)cpio_addr,1);
    }
}

int main(){
    // register unsigned long dtb_reg asm ("x15");

    init_uart();
    init_uart_buf();
    init_timer();
    init_taskq();
    init_frame_freelist();
    *AUX_MU_IER_REG = 1; // 3 for RX, TX interrupt enable
    *IRQ_ENABLE1 = 1<<29;

    // writes_uart("\r\n");
    // writes_uart("██╗    ██╗███████╗██╗      ██████╗ ██████╗ ███╗   ███╗███████╗\r\n");
    // writes_uart("██║    ██║██╔════╝██║     ██╔════╝██╔═══██╗████╗ ████║██╔════╝\r\n");
    // writes_uart("██║ █╗ ██║█████╗  ██║     ██║     ██║   ██║██╔████╔██║█████╗  \r\n");
    // writes_uart("██║███╗██║██╔══╝  ██║     ██║     ██║   ██║██║╚██╔╝██║██╔══╝  \r\n");
    // writes_uart("╚███╔███╔╝███████╗███████╗╚██████╗╚██████╔╝██║ ╚═╝ ██║███████╗\r\n");
    // writes_uart("╚══╝╚══╝ ╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝\r\n");
    // uart_buf_writes_push("\r\n");
    uart_buf_writes_push("██╗    ██╗███████╗██╗      ██████╗ ██████╗ ███╗   ███╗███████╗\r\n");
    uart_buf_writes_push("██║ m  ██║██╔════╝██║     ██╔════╝██╔═══██╗████╗ ████║██╔════╝\r\n");
    uart_buf_writes_push("██║ █╗ ██║█████╗  ██║     ██║     ██║   ██║██╔████╔██║█████╗  \r\n");
    uart_buf_writes_push("██║███╗██║██╔══╝  ██║     ██║     ██║   ██║██║╚██╔╝██║██╔══╝  \r\n");
    uart_buf_writes_push("╚███╔███╔╝███████╗███████╗╚██████╗╚██████╔╝██║ ╚═╝ ██║███████╗\r\n");
    uart_buf_writes_push(" ╚══╝╚══╝ ╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝\r\n");
    // uart_buf_writes_push("Hello World!\r\n");
    // asm volatile(
    //     "bl core_timer_enable\n\t"
    //     // "bl core_timer_handler\n\t"
    // );
    
    // unsigned long long cur_excep_lvl;
    // asm volatile(
    //     "mrs x0, CurrentEL\n\t"
    //     "mov %0,x0\n\t"
    //     :"=r" (cur_excep_lvl)
    // );
    // writes_uart("Current exceotion level:");
    // writehex_uart(cur_excep_lvl>>2,1);

    // writes_uart("Loaded dtb address: ");
    // writehex_uart(dtb_reg);
    // writes_uart("\r\n");
    
    fdt_traverse(initramfs_callback);

    writes_uart("Malloc string at address: ");
    writehex_uart((unsigned long)&_head_start_brk,1);

    // char* string = simple_malloc(8);
    // string[0]=1;
    // string[1]=2;
    // string[2]=3;
    // string[3]=4;
    // string[4]=5;     
    // add_timer(writes_nl_uart,"HELLO1",5);
    // add_timer(writes_nl_uart,"HELLO2",7);
    // add_timer(writes_nl_uart,"HELLO3",10);

    int got_freeaddr[10];
    for (int i = 0; i < 10; i++)
    {
        unsigned int* f_addr;
        f_addr = get_freeframe_addr(0x3000);
        got_freeaddr[i]=f_addr;
        writes_uart("Got address ");
        writehex_uart((unsigned int)f_addr,TRUE);
    }
    for (int i = 0; i < 10; i++)
    {
        writes_uart("Free address ");
        writehex_uart(got_freeaddr[i],TRUE);
        free_frame(got_freeaddr[i]);
    }
    
    // unsigned int* f_addr;
    // f_addr = get_freeframe_addr(0x1000);
    // writes_uart("Got address ");
    // writehex_uart((unsigned int)f_addr,TRUE);
    // f_addr = get_freeframe_addr(0x1000);
    // writes_uart("Got address ");
    // writehex_uart((unsigned int)f_addr,TRUE);
    
    // unsigned int* f_addr;
    // f_addr = get_freeframe_addr(8192);
    // writes_uart("Got address ");
    // writehex_uart((unsigned int)f_addr,TRUE);
    // free_frame(0x10000000);

    while(1)
    { 
        // writes_uart("main\r\n");
        read_command();
        //read_uart_buf();
    }
    return 0;
}
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

// uint32_t* cpio_addr;
// bool initramfs_callback(fdt_prop* prop,char * name,uint32_t len_prop){
//     if(strncmp(name,"linux,initrd-start",18)==0){ // start address of initrd
//         writes_uart("Found target: \"");
//         writes_n_uart(name,18);
//         writes_uart("\" at address ");
//         cpio_addr = (unsigned int*)((unsigned long)big2little(*((uint32_t*)(prop+1))));
//         writehex_uart((unsigned long)cpio_addr,TRUE);
//         return TRUE;
//     }
//     else if(strncmp(name,"linux,initrd-end",18)==0)
//     {
//         writes_uart("Found target: \"");
//         writes_n_uart(name,18);
//         writes_uart("\" at address ");
//         cpio_addr = (unsigned int*)((unsigned long)big2little(*((uint32_t*)(prop+1))));
//         writehex_uart((unsigned long)cpio_addr,TRUE);
//         return TRUE;
//     }
//     return FALSE;
// }

int main(){
    // register unsigned long dtb_reg asm ("x15");
    
    //init_frame_freelist();
    init_uart();
    init_uart_buf();
    init_taskq();
    init_memory();
    init_timer();
    
    
    *AUX_MU_IER_REG = 1; // 3 for RX, TX interrupt enable
    *IRQ_ENABLE1 = 1<<29;

    writes_uart("\r\n");
    writes_uart("██╗    ██╗███████╗██╗      ██████╗ ██████╗ ███╗   ███╗███████╗\r\n");
    writes_uart("██║    ██║██╔════╝██║     ██╔════╝██╔═══██╗████╗ ████║██╔════╝\r\n");
    writes_uart("██║ █╗ ██║█████╗  ██║     ██║     ██║   ██║██╔████╔██║█████╗  \r\n");
    writes_uart("██║███╗██║██╔══╝  ██║     ██║     ██║   ██║██║╚██╔╝██║██╔══╝  \r\n");
    writes_uart("╚███╔███╔╝███████╗███████╗╚██████╗╚██████╔╝██║ ╚═╝ ██║███████╗\r\n");
    writes_uart("╚══╝╚══╝ ╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝\r\n");
    
    // writes_uart("Hello World!\r\n");
    
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
    
    // fdt_traverse(initramfs_start_callback);
    // fdt_traverse(initramfs_end_callback);

    // list_freeChunkNode();
    // list_freeFrameNode();
    
    // unsigned long long got_freeaddr[16];
    // for (int i = 0; i < 16; i++)
    // {
    //     unsigned int* f_addr;
    //     writes_uart("Mallocing ");
    //     write_int_uart(power(2,i),TRUE);
    //     f_addr = my_malloc(power(2,i));

    //     got_freeaddr[i]=f_addr;
    //     writes_uart("Got address ");
    //     writehex_uart((unsigned int)f_addr,TRUE);
    //     // list_freeFrameNode();
    // }
    // list_freeChunkNode();
    // list_freeFrameNode();

    // for (int i = 0; i < 16; i++)
    // {
    //     writes_uart("Free address ");
    //     writehex_uart(got_freeaddr[i],TRUE);
    //     free(got_freeaddr[i]);
    // }
    // list_freeChunkNode();
    // list_freeFrameNode();
    
    while(1)
    { 
        // writes_uart("main\r\n");
        read_command();
        //read_uart_buf();
    }
    return 0;
}
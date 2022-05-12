#include <uart.h>
#include <shell.h>
#include <cpio.h>
#include <malloc.h>
#include <string.h>
#include <fdt.h>
#include <irq.h>
#include <allocator.h>
#include <math.h>


int main(unsigned long dtb_base){
    uart_init();
    print_string(UITOHEX, "[*] DTB_BASE: 0x", dtb_base, 1);
    fdt_traverse((fdt_header *)dtb_base, initramfs_callback);
    all_allocator_init();    
    // buddy_debug();
    // chunk_debug();
    kmalloc_debug();

    // uart_getc();
    // enable_timer_irq();
    // enable_AUX_MU_IER_r();
    // enable_irq(); // DAIF set to 0b0000
    

    PrintWelcome();

    ShellLoop();
    
    return 0;
}
#include "shell.h"
#include "uart.h"
#include "mbox.h"
#include "lib.h"
#include "reboot.h"

void shell(){
    char cmd[BUFFER_SIZE];
    int ret;
    int reboot_scheduled = 0;

    shell_prompt();
    while(1){
        uart_puts("# ");
        uart_gets(cmd);
        // uart_puts(cmd);
        // uart_send('n');
        // uart_send('\n');
        
        ret = strcmp(cmd, "help");
        // uart_int(ret);
        // uart_puts("\n");

        if ( ret == 0 ) {
            help();
            continue;
        }

        ret = strcmp(cmd, "hello");
        // uart_int(ret);
        // uart_puts("\n");
        
        if ( ret == 0  ) {
            hello_world();
            continue;
        }
        
        ret = strcmp(cmd, "clear");
        // uart_int(ret);
        // uart_puts("\n");
        
        if ( ret == 0 ) {
            uart_puts("\033[2J\033[H");
            continue;
        }

        ret = strcmp(cmd, "reboot");
        // uart_int(ret);
        // uart_puts("\n");
        
        if ( ret == 0 ) {
            reboot_scheduled = 1;
            reboot();
            continue;
        }

        ret = strcmp(cmd, "reboot -c");
        // uart_int(ret);
        // uart_puts("\n");
        
        if ( ret == 0 ) {
            if (reboot_scheduled) {
                cancel_reboot();
            }
            else {
                uart_puts("No scheduled reboot.\n");
            }
            continue;
        }
        
        ret = strcmp(cmd, "");
        // uart_int(ret);
        // uart_puts("\n");
        
        if ( ret == 0 ) {
            continue;
        }

        uart_puts("command not found.\n");
    }
}

void shell_prompt(){
    uart_puts("\033[2J\033[H");
    unsigned int board_revision;
    get_board_revision(&board_revision);
    uart_puts("Board revision is : 0x");
    uart_hex(board_revision);
    uart_puts("\n");
    
    unsigned int arm_mem_base_addr;
    unsigned int arm_mem_size;

    get_arm_memory_info(&arm_mem_base_addr,&arm_mem_size);
    uart_puts("ARM memory base address in bytes : 0x");
    uart_hex(arm_mem_base_addr);
    uart_puts("\n");
    uart_puts("ARM memory size in bytes : 0x");
    uart_hex(arm_mem_size);
    uart_puts("\n");

    uart_puts("\n");
    uart_puts("This is a simple shell for raspi3.\n");
    uart_puts("type help for more information\n");
}

void help(){
    uart_puts("help     : print this help menu.\n");
    uart_puts("hello    : print hello world!\n");
    uart_puts("clear    : clear screen.\n");
    uart_puts("reboot   : reboot raspberry pi.\n");

}

void hello_world(){
    uart_puts("hello world!\n");
}

void reboot(){
    // char buf[BUFFER_SIZE];
    // int time;
    // uart_puts("time out: ");
    // uart_gets(buf);
    // time = atoi(buf);
    // uart_puts("Set time out ");
    // uart_int(time);
    // uart_puts(" sec.\n");
    // reset(time);

    uart_puts("You have roughly 17 seconds to cancel reboot.\nCancel reboot with\nreboot -c\n");
    reset(0);
}

void cancel_reboot(){
    cancel_reset();
    uart_puts("reboot canceled.\n");
}
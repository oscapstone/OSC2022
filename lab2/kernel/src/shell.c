#include "shell.h"
#include "mailbox.h"
#include "string.h"
#include "reboot.h"
#include "uart.h"
#include "cpio.h"
#include "dtb.h"

#define CMD_LEN 128

extern unsigned long dtb_addr;

void init_cpio_addr()
{
    fdt_traverse(initramfs_callback);
}

void shell_init(){
    uart_init();
    uart_puts("Welcome!\n");
    register unsigned long x20 asm("x20");
	dtb_addr = x20;
    // dtb_addr = 0x8200000;
    init_cpio_addr();
}

void shell_cmd(char *cmd){
    if(strcmp(cmd, "help")){
        uart_puts("help\t: print this help menu\r\n");
        uart_puts("hello\t: print Hello World!\r\n");
        uart_puts("devinfo\t: print device info\r\n");
        uart_puts("reboot\t: reboot the device\r\n");
        uart_puts("ls\t: list files\r\n");
        uart_puts("cat\t: display content of a file\r\n");
    }
    else if(strcmp(cmd, "hello")){
        uart_puts("Hello World!\r\n");
    }
    else if(strcmp(cmd, "devinfo")){
        get_board_revision();
        get_memory_info();
    }
    else if(strcmp(cmd, "reboot")){
        uart_puts("Waiting for reboot ...\r\n");
        reset(100);
    }
    else if(strcmp(cmd, "ls")){
        ls();
    }
    else if(strcmp(cmd, "cat")){
        uart_puts("Filename: \r\n");
        char filepath[128];
        char in;
        int i = 0;
        while(1) {
            in = uart_getc();

            if(in == '\n'){
                uart_puts("\r\n");
                cat(filepath);
                break;
            }
            else{
                filepath[i] = in;
                uart_send(in);
                i++;
            }
        }
    }
    else{
        uart_puts("Command not found.\r\n");
    }
    
}

void shell_run(){
    char in;
    char cmd[CMD_LEN];
    int end = 0;
    uart_puts("> ");
    while(1) {
        in = uart_getc();

        if(in == '\n'){
            cmd[end] = '\0';
            uart_puts("\r\n");
            shell_cmd(cmd);
            uart_puts("> ");
            end = 0;
        }
        else{
            cmd[end] = in;
            uart_send(in);
            end++;
        }
    }
}

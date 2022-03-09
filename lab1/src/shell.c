#include "shell.h"
#include "io.h"
#include "mailbox.h"
#include "string.h"
#include "reboot.h"
#include "uart.h"

#define CMD_LEN 128

void shell_init(){
    io_init();
    puts("Welcome!\n");
}

void shell_cmd(char *cmd){
    if(strcmp(cmd, "help")){
        puts("help\t: print this help menu\r\n");
        puts("hello\t: print Hello World!\r\n");
        puts("devinfo\t: print device info\r\n");
        puts("reboot\t: reboot the device\r\n");
    }
    else if(strcmp(cmd, "hello")){
        puts("Hello World!\r\n");
    }
    else if(strcmp(cmd, "devinfo")){
        get_board_revision();
        get_memory_info();
    }
    else if(strcmp(cmd, "reboot")){
        puts("Waiting for reboot ...\r\n");
        reset(100);
    }
    else{
        puts("Command not found.\r\n");
    }
    
}

void shell_run(){
    char in;
    char cmd[CMD_LEN];
    int end = 0;
    while(1) {
        in = get_user_inputc();

        if(in == '\n'){
            cmd[end] = '\0';
            putc(in);
            shell_cmd(cmd);
            end = 0;
        }
        else{
            cmd[end] = in;
            putc(in);
            end++;
        }
    }
}

#include "shell.h"
#include "string.h"
#include "mailbox.h"
#include "reboot.h"
#include "bootload.h"
int match_command(char *buffer){
    if(strcmp(buffer,"help")==0){
        return help;
    }
    else if(strcmp(buffer,"hello")==0){
        return hello;
    }else if(strcmp(buffer,"revision")==0){
        return revision;
    }else if(strcmp(buffer,"memory")==0){
        return memory;
    }
    else if(strcmp(buffer,"reboot")==0){
        return reboot;
    }
    else if(strcmp(buffer,"ccreboot")==0){
        return ccreboot;
    }
    else if(strcmp(buffer,"bootload")==0){
        return bootload;
    }
    else{
        return unknown;
    }
}

void read_command(){
    char buffer[25600];
    int count=0;
    int action = 0;
    writes_uart("# ");
    while(1){
        char c = read_uart();
        
        if(c!='\n' && count<256){
            if(c>=0 && c<=127){ // only accept the value is inside 0~127(ASCII).
               writec_uart(c);
                buffer[count++] = c; 
            }
            
        }else{
            writec_uart('\r');
            writec_uart('\n');
            buffer[count]='\0';
            if(buffer[0] != '\0'){
                action = match_command(buffer);
                handle_command(action, buffer);
            }
            break;
        }
        
    }
    return;
}
void handle_command(enum Action action, char *buffer){
    switch (action)
    {
    case help:
        writes_uart("help       : print this help menu\r\n");
        writes_uart("hello      : print Hello World!\r\n");
        writes_uart("revision   : print board revision\r\n");
        writes_uart("memory     : print ARM memory address and size\r\n");
        writes_uart("reboot     : reboot the device after 50000 ticks\r\n");
        writes_uart("ccreboot   : cancel reboot the device\r\n");
        writes_uart("bootload   : boot the kernel image from uart\r\n");
        break;
    case hello:
        writes_uart("Hello World!\r\n");
        break;
    case revision:
        get_board_revision();
        break;
    case memory:
        get_ARM_memory();
        break;
    case reboot:
        writes_uart("reset after 50000 ticks\r\n");
        reset(50000);
        break;
    case ccreboot:
        writes_uart("canceling reset operation...\r\n");
        cancel_reset();
        writes_uart("reset operation has been canceled\r\n");
        break;
    case bootload:
        writes_uart("Start loading the image file.\r\n");
        bootload_image();
        break;
    default:
        writes_uart("command not found: ");
        writes_uart(buffer);
        writes_uart("\r\n");
        break;
    }
    
    return;
}
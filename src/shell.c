#include "uart.h"
#include "string.h"
#include "command.h"
#include "shell.h"

void shell_start(){
    unsigned int size = 0;
    char buffer[MAX_BUFFER_LEN];
    while(1){
        uart_puts("# ");
        memset(buffer, 0, MAX_BUFFER_LEN);
        size = readline(buffer, MAX_BUFFER_LEN);
        if(!size)
            continue;
        if(!strcmp("help", buffer))
            cmd_help();
        else if(!strcmp("hello", buffer))
            cmd_hello();
        else if(!strcmp("revision", buffer))
            cmd_revision();
        else if(!strcmp("memory", buffer))
            cmd_memory();
        else if(!strcmp("reboot", buffer))
            cmd_reboot();
        else
            cmd_err(buffer);
    }
}
#include "commands.h"

#define BUFFER_LEN 100


void read_command(char* buffer){
    uart_puts("\r\n# ");
    int idx=0;
    while(1){
        if(idx >= BUFFER_LEN) break;
        //char c = uart_getc();
        char c = uart_async_getc();
        if(c=='\n') {
            uart_puts("\r\n"); // echo
            break;
        }
        else {
            buffer[idx++] = c;
        }
        uart_send(c); // echo
    }
}

void clear_buffer(char* buffer, int size){
    for(int i = 0; i<size; i++){
        buffer[i] = '\0';
    }
}


void execute_command(const char* cmd){
    int cmd_len = sizeof(cmd_list)/sizeof(commads);
    uart_puts("\0"); // I'dont konw why if you remove this bugs occurs
    int command_found = 0;
    for(int i=0; i<cmd_len; i++){
        if(compare(cmd, cmd_list[i].cmd) == 1){
            command_found = 1;
            cmd_list[i].func();
            break;
        }
    }
    if(command_found == 0){
        uart_puts("command not found");
    }
}

void main()
{
    char input_buffer[BUFFER_LEN];

    // set up serial console
    uart_init();
    // say hello
    uart_puts("Wellcome!\r\n");
    // enables interrupt in EL1 (for uart async IO)
    enable_interrupt(); // msr DAIFClr, 0xf

    while(1) {
        // echo everything back
        clear_buffer(input_buffer, BUFFER_LEN);
        // read command
        read_command(input_buffer);
        // uart_puts(input_buffer); // echo input
        execute_command(input_buffer);
    }
}

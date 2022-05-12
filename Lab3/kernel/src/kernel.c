#include "commands.h"

#define BUFFER_LEN 1000

void read_command(char* buffer){
    uart_puts("\r\n# ");
    int idx=0;
    while(1){
        if(idx >= BUFFER_LEN) break;
        //char c = uart_getc();
        char c = uart_getc();
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

void parse_command(char* cmd){
    
}

void execute_command(const char* cmd){
    int cmd_len = sizeof(cmd_list)/sizeof(commads);
    uart_puts("\0"); // I'dont konw why if you remove this bugs occurs
    int command_found = 0;
    for(int i=0; i<cmd_len; i++){
        if(find_command(cmd, cmd_list[i].cmd) == 1){
            command_found = 1;
            char *args;
            int args_start = strlen(cmd_list[i].cmd) + 1;
            if(args_start > strlen(cmd)){
                // command with no argument
                args = "foo";
            } else {
                args = &cmd[args_start];
            }
            
            cmd_list[i].func(args);
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
    // buddy system init
    buddy_init();
    // say hello
    uart_puts("Wellcome!\r\n");
    // enables interrupt in EL1 (for uart async IO)
    enable_interrupt(); // msr DAIFClr, 0xf

    head_event = 0;

    while(1) {
        // echo everything back
        clear_buffer(input_buffer, BUFFER_LEN);
        // read command
        read_command(input_buffer);
        // uart_puts(input_buffer); // echo input
        execute_command(input_buffer);
    }
}

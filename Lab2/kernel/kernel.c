#include "commands.h"

#define BUF_LEN 100
#define CPIO_LOC 0x8000000



void read_command(char* buffer){
    uart_puts("\r\n# ");
    int idx=0;
    while(1){
        if(idx >= BUF_LEN) break;
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

unsigned long __stack_chk_guard;
void __stack_chk_guard_setup(void)
{
     __stack_chk_guard = 0xBAAAAAAD;//provide some magic numbers
}

void __stack_chk_fail(void)                         
{
 /* Error message */                                 
}// will be called when guard variable is corrupted 


void *memcpy(void *dest, const void *src, unsigned int n)
{
    for (unsigned int i = 0; i < n; i++)
    {
        ((char*)dest)[i] = ((char*)src)[i];
    }
}

void main()
{
    // set up serial console
    uart_init();
    
    // say hello
    uart_puts("Wellcome!\r\n");
    
    char input_buffer[BUF_LEN];
    while(1) {
        // uart_puts("Wellcome!\r\n");
        // echo everything back
        clear_buffer(input_buffer, BUF_LEN);
        // read command
        read_command(input_buffer);
        // uart_puts(input_buffer); // echo input
        execute_command(input_buffer);
    }
}

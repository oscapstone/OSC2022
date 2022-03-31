#include "shell.h"
#include "string.h"
#include "uart.h"
#include "system.h"
#include "dtb_parser.h"



void shell_welcome(){
    uart_puts("\r\n------------------------\r\n");
    uart_puts("=        Kernel        =\r\n");
    uart_puts("------------------------\r\n");
    char *tmp2 = (char*)simple_malloc(10);
    tmp2[0] = 'a';
    tmp2[1] = 'p';
    tmp2[2] = 'p';
    tmp2[3] = 'l';
    tmp2[4] = 'e';
    tmp2[5] = '\0';
    uart_hex((unsigned int)tmp2);
    uart_puts("\n");
    uart_puts(tmp2);
    uart_hex(INITRD_ADDR);
    uart_puts("\n");
}


void shell() {
    while(uart_get() == '\0'){}
	while(uart_get() == '\0'){}
    shell_welcome();

    uart_puts(USER_NAME);
    uart_puts("@");
    uart_puts(MACHINE_NAME);
	uart_puts("$ ");

    char input_buffer[MAX_BUFFER_LEN];
    char *buf_ptr = input_buffer;
    memset (buf_ptr, '\0', MAX_BUFFER_LEN);   
    char c;
    while(1){
        c = uart_get();
        if(c == '\r'){
            uart_puts("\r\n");
            system_command(input_buffer);
            
			uart_puts(USER_NAME);
			uart_puts("@");
			uart_puts(MACHINE_NAME);
			uart_puts("$ ");
            buf_ptr = input_buffer;
            memset(buf_ptr, '\0', MAX_BUFFER_LEN);
        }
        else if(c == '\b'){
            if(buf_ptr > input_buffer){
                uart_puts("\b \b");
                *(--buf_ptr) = '\0';
            }
        }
        else{
            uart_send(c);
            *buf_ptr++ = c;
        }
    }


}


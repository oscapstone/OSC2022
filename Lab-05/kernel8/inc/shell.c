#include "shell.h"


void *memset(void *s, int c, unsigned long n){
    char *xs = s;
    while (n--)
        *xs++ = c;
    return s;
}

void shell_welcome(){
    uart_puts("\r\n------------------------\r\n", 100);
    uart_puts("=        Kernel        =\r\n", 100);
    uart_puts("------------------------\r\n", 100);
}


void clear_buffer() {
  
  buffer_index = 0;
  for (int i = 0; i < MAX_BUFFER_LEN; i++) {
    input_buffer[i] = '\0';
  }
}

void shell() {
	


	while(uart_get() == '\0');
	while(uart_get() == '\0');

    shell_welcome();
    uart_puts(USER_NAME, 100);
    uart_puts("@", 100);
    uart_puts(MACHINE_NAME, 100);
	uart_puts("$ ", 100);

    char input_buffer[MAX_BUFFER_LEN];
    char *buf_ptr = input_buffer;
    memset (buf_ptr, '\0', MAX_BUFFER_LEN);   
    char c;


    while(1){


		c = uart_get();
		
		if(c == '\r'){
			uart_puts("\r\n", 100);
			memset (buf_ptr, '\0', MAX_BUFFER_LEN);   
		    system_command(input_buffer);
			uart_puts(USER_NAME, 100);
			uart_puts("@", 100);
			uart_puts(MACHINE_NAME, 100);
			uart_puts("$ ", 100);
			buf_ptr = input_buffer;
			memset(buf_ptr, '\0', MAX_BUFFER_LEN);
		}
		else if(c == '\b'){
			if(buf_ptr > input_buffer){
				uart_puts("\b \b", 100);
				*(--buf_ptr) = '\0';
			}
		}
	    else{
	        uart_send(c);
	        *buf_ptr++ = c;
        }



		
    }


}


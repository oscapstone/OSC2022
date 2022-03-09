#include "../include/UART.h"
#include "../include/lib.h"
#include "../include/mailbox.h"
#include "../include/reboot.h"

void main(){
	UART_init();
	//UART_read();
	UART_put("\r\ntest\r\n");

	Board_revision();
	UART_put("Board revision : ");
	UART_hex(mailbox[5]);
	UART_put("\r\n");

	Mem_add_size();
	UART_put("ARM memory base address : ");
	UART_hex(mailbox[5]);
	UART_put("\r\n");
	UART_put("ARM memory size : ");
	UART_hex(mailbox[6]);
	UART_put("\r\n");

	char str[10] = "";
	int idx = 0;

	while(1){
		char c = UART_read();
		UART_write(c);

		if(c != '\n')
			str[idx++] = c;
		else{
			if(!Strncmp(str, "help", 4))
				UART_put("help      : print this help menu\nhello     : print Hello World!\nreboot    : reboot the device\r\n");
			else if(!Strncmp(str, "hello", 5))
				UART_put("Hello World!\r\n");
			else if(!Strncmp(str, "reboot", 6)){
				//UART_put("\n");
				reset(1);
			}
			else
				UART_put("No such command!\n");

			for(int i=0;i<10;i++)
				str[i] = 0;
			idx = 0;
		}
	}
}
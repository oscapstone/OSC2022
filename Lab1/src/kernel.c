#include "mini_uart.h"
#include "reboot.h"
#include "mailbox.h"
#include "printf.h"
#include "utils.h"


void kernel_main(void)
{
	uart_init();
	init_printf(0, putc);
	int el = get_el();
	printf("Exception level: %d \r\n", el);

	int counter = 0, command_index = 0;
    char sentence[7]={};
    char COMMAND[4][7] = {"help\n\n\n", "hello\n\n","reboot\n","mailbox"};
	
	uart_send_string("# Startup the rpi3\r\n");
	uart_send_string("# ");

	while (1) {
		char input = uart_recv();
		if(input != '\r')
			uart_send(input);
		else
			uart_send_string("\r\n");

		if(input != '\r'){
            if(counter<7){
                sentence[counter] = input;
                counter += 1;
            }
            else{
                sentence[0]='\n';
            }
        }
        else{
            if(counter == 4)
                command_index = 0;
            else if(counter == 5)
                command_index = 1;
			else if (counter == 6)
				command_index = 2;
			else if (counter == 7)
				command_index = 3;
            else
                command_index = -1;
			int CMD_flag = 1;
            if(command_index != -1){
                for (int i = 0; i < counter; i ++) 
            	    if(sentence[i] != COMMAND[command_index][i])
            	        CMD_flag = 0;
			}			
			if(CMD_flag == 1 && command_index != -1){
				if(command_index == 0){
					uart_send_string("help      : print this help menu\r\n");
					uart_send_string("hello     : print Hello World!\r\n");
					uart_send_string("reboot    : print reboot the device\r\n");
					uart_send_string("mailbox   : print  board revision and ARM memory base address and size\r\n");
				}
				else if(command_index == 1)
					uart_send_string("Hello World!\r\n");
				else if(command_index == 2){
					uart_send_string("reboot the device\r\n");
					reset(3000);
				}
				else if(command_index == 3){
					//get_serial();
					get_board_revision();
					get_arm_memory_info();
				}

    	    }
			//else
			//	uart_send_string("Illegal command\r\n");
			

            counter = 0;
            for (int i = 0; i < 5; i ++)
                sentence[i] = '\n';
			uart_send_string("# ");
        }	

	}
}

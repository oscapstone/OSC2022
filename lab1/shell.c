
#include "shell.h"
#include "string.h"
#include "command.h"
#include "uart.h"

void shell_start(){
	int buffer_counter = 0;
	char input_char;
	char buffer[MAX_BUFFER_LEN];

	uart_puts("# ");
	while(1){
		input_char = uart_getc();
		command_proccess(input_char, buffer, &buffer_counter);
	}
}

void command_proccess(char input_char, char *buffer, int *buffer_counter){
	enum CHAR_CLASS input_anls = analysis(input_char);
    if(*buffer_counter == MAX_BUFFER_LEN){
        return;
    }
	
	if(input_anls == UNKNOWN){
		return;
	}else if(input_anls == BACKSPACE){
		if(*buffer_counter > 0){
			(*buffer_counter)--;
		}
		uart_send('q');
	}else if(input_anls == NEW_LINE){
        buffer[(*buffer_counter)] = '\0';
        uart_send(input_char);
        if(!strcmp(buffer,"help")){
            command_help();
        }else if(!strcmp(buffer,"hello")){
            command_hello();
        }else if(!strcmp(buffer,"mailbox")){
            command_mailbox();
        }else if(!strcmp(buffer,"reboot")){
            command_reboot();
        }else{
            command_notfound(buffer);
        }
        *buffer_counter = 0;
        uart_puts("# ");
	}else if(input_anls == NORMAL_CHAR){
        buffer[(*buffer_counter)] = input_char;
		(*buffer_counter)++;
        uart_send(input_char);
	}
}

enum CHAR_CLASS analysis(char input_char){
	if(input_char<0 || input_char>127){
		return UNKNOWN;
	}else if(input_char == BACKSPACE){
		return BACKSPACE;
	}else if(input_char == LINE_FEED || input_char == CARRIAGE_RETURN ){
		return NEW_LINE;
	}else
		return NORMAL_CHAR;
}

#include "shell.h"
#include "string.h"
#include "command.h"
#include "uart.h"
#include "cpio.h"
#include "dtb_parse.h"

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
		char *arg1_idx = "";
		for(int i=0;buffer[i]!='\0';i++){
			if(buffer[i]==' '){
				buffer[i] = '\0';
				arg1_idx = buffer + i +1;
				break;
			}
		}
        uart_send(input_char);
        if(!strcmp(buffer,"help")){
            command_help();
        }else if(!strcmp(buffer,"ls")){
            cpio_ls();
        }else if(!strcmp(buffer,"parse_dtb_header")){
            parse_header();
        }else if(!strcmp(buffer,"cat")){
            cpio_cat(arg1_idx);
        }else if(!strcmp(buffer,"alloc_test")){
            alloc_test();
        }else if(!strcmp(buffer,"parse_dtb")){
            parse_dtb();
        }else if(!strcmp(buffer,"kernel_load")){
            kernel_load();
        }else{
            command_notfound(buffer);
        }
		uart_puts("# ");
        *buffer_counter = 0;
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
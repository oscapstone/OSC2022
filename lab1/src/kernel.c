#include "mini_uart.h"
#include "printf.h"
#include "mailbox.h"
#include "reboot.h"

void kernel(void)
{
	uart_init();
	init_printf(0, putc);

	char word=0;
	char str[10]={};
	while (1) {
		unsigned int len=0;
		uart_send_string("# ");
		while(1){
			word=uart_recv();
			uart_send(word);
			if(word=='\r'){
				uart_send('\n');
				break;		
			}				
			str[len]=word;
			if(word==127){
				len-=2;
			}			
			len++;
		}
		str[len]='\0';
		//uart_send_string(str);
		//word=uart_recv();
		//uart_send(word);
		if(str[0]=='h' && str[1]=='e' && str[2]=='l' && str[3]=='p' && str[4]=='\0'){
			uart_send_string("help    : print this help menu\r\n");
			uart_send_string("hello   : print Hello World!\r\n");
			uart_send_string("reboot  : reboot thr device\r\n");
		}
		else if(str[0]=='h' && str[1]=='e' && str[2]=='l' && str[3]=='l' && str[4]=='o' && str[5]=='\0'){
			uart_send_string("Hello World!\r\n");
		}
		else if(str[0]=='e' && str[1]=='x' && str[2]=='3' && str[3]=='\0'){
			printf("board revision:");
			mail_message_t msg;
			mailbox[0]=28;
			mailbox[1]=0x00000000;
			mailbox[2]=0x00010002;
			mailbox[3]=4;
			mailbox[4]=0x00000000;
			mailbox[5]=0;
			mailbox[6]=0x00000000;
			unsigned int addr = (unsigned int)(((unsigned long)mailbox & ~0xF) | (8 & 0xF));
			mailbox_send(addr,8);
			//printf("0x%x\r\n",addr);
			msg=mailbox_read(8);
			printf("0x%x\r\n",mailbox[5]);
			
			printf("ARM base address:");
			mailbox[0]=32;
			mailbox[1]=0x00000000;
			mailbox[2]=0x00010005;
			mailbox[3]=8;
			mailbox[4]=8;
			mailbox[5]=0;
			mailbox[6]=0x00000000;
			addr = (unsigned int)(((unsigned long)mailbox & ~0xF) | (8 & 0xF));
			mailbox_send(addr,8);
			//printf("0x%x\r\n",addr);
			msg=mailbox_read(8);
			printf("0x%x\r\n",mailbox[5]);
			printf("ARM size:");
			printf("0x%x\r\n",mailbox[6]);
		}
		else if(str[0]=='r' && str[1]=='e' && str[2]=='b' && str[3]=='o' && str[4]=='o' && str[5]=='t' && str[6]=='\0'){
			reset(2000);
		}
	}

}


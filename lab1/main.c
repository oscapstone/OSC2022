#include "inc/uart.h"
#include "inc/reboot.h"
#include "inc/mbox.h"

void printinfo()
{
    // get the board's unique serial number with a mailbox call
    mbox[0] = 7 * 4;                  // buffer size in bytes
    mbox[1] = REQUEST_CODE;
    mbox[2] = GET_BOARD_REVISION;     // tag identifier 
    mbox[3] = 4;                      // maximum of request and response value buffer's length.
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0;                      // value buffer
	// tags end
    mbox[6] = END_TAG;

    // send the message to the GPU and receive answer
    mbox_call(MBOX_CH_PROP);
    uart_puts("My board revision is: 0x");
    uart_hex(mbox[5]);
    uart_puts("\n");

    // get the board's unique serial number with a mailbox call
    mbox[0] = 8 * 4;               // length of the message
    mbox[1] = REQUEST_CODE;        // this is a request message
    mbox[2] = GET_ARM_MEMORY;      // get serial number command
    mbox[3] = 8;                   // buffer size
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0;                   // clear output buffer
    mbox[6] = 0;
    mbox[7] = END_TAG;

    // send the message to the GPU and receive answer
    mbox_call(MBOX_CH_PROP);
	uart_puts("My memory base address is: 0x");
	uart_hex(mbox[5]);
	uart_puts("\n");
	uart_puts("My memory size is: 0x");
	uart_hex(mbox[6]);
	uart_puts("\n");
}


void shell(){
	uart_puts("Welcome!\n");
	char buffer[1000];
	
	int cnt;
	while(1){
		//uart_puts("\n $ ");

		for(int i = 0;i < sizeof(buffer);i++){
			buffer[i] = 0;
		}
		uart_puts("$ ");

		cnt=0;
		do{
			buffer[cnt++]=uart_getc();
			uart_send(buffer[cnt - 1]);
		}while(buffer[cnt - 1] != '\r');//enter
		buffer[--cnt]=0;

		//uart_puts("++++++++++++++++++++");


		if(strcmp(buffer,"help") == 0){
			uart_puts("commands:\n");
			uart_puts("          help\n");
			uart_puts("          hello\n");
			uart_puts("          reboot\n");
			uart_puts("          printinfo\n");
		}else if(strcmp(buffer, "hello") == 0){
			uart_puts("Hello World!\n");
		}else if(strcmp(buffer, "reboot") == 0){
			uart_puts("rebooting...\n");
			reset(100);
		}else if(strcmp(buffer, "printinfo") == 0){
			printinfo();
		}else{
			uart_puts("Error : No such command \"");
			uart_puts(buffer);
			uart_puts("\".\n");
		}
	}
}


void main(){
	uart_init();
	shell();
}
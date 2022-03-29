#include "../include/UART.h"

int boot_main(){
	volatile unsigned long dtb_base;
	asm volatile ("mov %0, x0" : "=r" (dtb_base));   // backup x0
	UART_init();
	UART_put("[Uartboot] Waiting for kernel image.\n\r");

	unsigned int kernel_size = 0;
	char* p = (char*)0x80000;

	char start_code[5]="hello";
    for(int i=0; i<5; i++){
        char b = UART_read();
		UART_write(b);
        if (b!=start_code[i]) 
            i--;
    }
	/*int i=1; 
	while(i<5){
		char data = UART_read();
		UART_write(data);
		if(data != i)
			i = (i==1?1:i-1);
		else
			i++;
	}*/
	UART_put("[Uartboot] Filtered\n\r");

	for(int i=0;i<4;i++){
		kernel_size = kernel_size<<8;
		kernel_size |= UART_read();
	}

	for(int i=0;i<kernel_size;i++)
		p[i] = UART_read();

	UART_put("[Uartboot] Received all kernel image.\n\r");

	asm volatile ("mov x1, %0" : : "r" (0x80000));    // %0 is input oprand, that is "0x80000"
	asm volatile ("mov x0, %0" : : "r" (dtb_base));   // load the original x0
    asm volatile ("br x1");   // branch to kernel

	return 0;
}
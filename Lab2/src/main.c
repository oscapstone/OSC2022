#include "../include/UART.h"
#include "../include/lib.h"
#include "../include/mailbox.h"
#include "../include/reboot.h"
#include "../include/cpio.h"
#include "../include/malloc.h"
#include "../include/dtb.h"

void print_help(){
	UART_put("\rhelp      : print this help menu.\n");
	UART_put("\rhello     : print Hello World.\n");
	UART_put("\rreboot	  : reboot the device.\n");
	UART_put("\rls		: list the files in cpio archive.\n");
	UART_put("\rcat		: display the specific file content.\n");
	UART_put("\rmalloc    : allocate a continuous space of memory of strings\n\r");
	return;
}

void main(){
	register unsigned long x0 asm("x0");
	unsigned long dtb_base = x0;

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
	
	fdt_traverse((fdt_header*)(dtb_base), initramfs_callback);

	char str[100];
	for(int i=0;i<100;i++)
		str[i] = 0;
	int idx = 0;
	
	//UART_put("# ");
	while(1){
		char c = UART_read();
		UART_write(c);

		if(c != '\n')
			str[idx++] = c;
		else{
			UART_write('\r');
			if(Strncmp(str, "help", 4)==0){
				print_help();
				UART_put("# ");
			}
			else if(Strncmp(str, "hello", 5)==0)
				UART_put("\rHello World!\r\n# ");
			else if(Strncmp(str, "reboot", 6)==0){
				UART_put("\r");
				reset(1);
			}
			else if(Strncmp(str, "ls", 2)==0){
				cpio_ls();
				UART_put("# ");
			}
			else if(Strncmp(str, "cat", 3)==0){
				cpio_cat();
				UART_put("# ");
			}
			else if(Strncmp(str, "malloc", 6)==0){
				unsigned int tmp = 7, num = 0, j=0;
				while(str[tmp] != ' ')
					tmp++;
				for(int i=7;i<tmp;i++){
					num *= 10;
					num += (str[i]-48);
				}
				
				unsigned int len = idx-tmp-1;
				//UART_hex(len);
				char s[len+1];
				for(int i=0;i<len;i++)
					s[i] = 0;
				for(int i=tmp+1;i<idx;i++)
					s[j++] = str[i];
				s[j] = '\n';

				char *ptr = simple_malloc(num);
				ptr = s;
				for(int i=0;i<=len;i++)
					UART_write(*(ptr+i));
			}
			else
				UART_put("No such command!\n");

			for(int i=0;i<100;i++)
				str[i] = 0;
			idx = 0;
		}
	}
}

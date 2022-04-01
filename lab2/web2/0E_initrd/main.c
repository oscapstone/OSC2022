
#include <stdint.h>
/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */
#include "malloc.h"
#include "uart.h"
#include "initrd.h"

// import our bitchunk from rd.o
extern volatile unsigned char _binary_ramdisk_start;

void main()
{
    // set up serial console
    uart_init();

    // list contents of an archive
    //initrd_list((char*)&_binary_ramdisk_start);
    char word;
    char str[10]={};
    // echo everything back
    while(1) {
	unsigned int len=0;
	uart_puts("# ");
	while(1){
		word=uart_getc();
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
	if(str[0]=='l'&& str[1]=='s' && str[2]=='\0'){
		//uart_puts("\n");
		//char *buf=INITRAMFS_ADDR;
		initrd_list((char*)&_binary_ramdisk_start);
		//initrd_list(buf);

	}
	else if(str[0]=='c'&& str[1]=='a' && str[2]=='t'&& str[3]=='\0'){		
		initrd_cat((char*)&_binary_ramdisk_start);

	}
	else if(str[0]=='m'&& str[1]=='a' && str[2]=='l'&& str[3]=='\0'){
		char *mal=simple_malloc(3);
		mal="hi\0";
		uart_puts(mal);		
		uart_puts("\n");
	}
    }
}

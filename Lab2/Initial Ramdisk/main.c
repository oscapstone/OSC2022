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

#include "uart.h"
#include "initrd.h"

// import our bitchunk from rd.o
extern volatile unsigned char _binary_ramdisk_start;

void main()
{
    // set up serial console
    uart_init();

	char sentence[20]={};
	int counter = 0, file_flag = 0;

	uart_puts("# ");
    // echo everything back
    while(1) {
		char input = uart_getc();
		if(input != '\n'){
			uart_send(input);
			if(counter<19){
                sentence[counter] = input;
                counter += 1;
            }
            else{
                sentence[0]='\n';
            }
		}
		else{
			uart_puts("\r\n");
			sentence[counter+1] = '\0';
			if (strcmp(sentence, "ls") == 0) {
				// list contents of an archive
   				initrd_list((char*)&_binary_ramdisk_start);
    		}
			if (file_flag==1){
				initrd_list_file((char*)&_binary_ramdisk_start, sentence);
			}

			if (strcmp(sentence, "cat") == 0){
				uart_puts("Filename: ");
				file_flag = 1;
			}
			else{
				file_flag = 0;
				uart_puts("# ");
			}

			for (int i = 0; i < 20; i ++)
                sentence[i] = '\0';

			counter = 0;
		}
    }



}

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
#include "mbox.h"

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

const unsigned char size = 256;
char buffer [256];

void set(long addr, unsigned int value) {
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void reset(int tick) {                 // reboot after watchdog timer expire
    set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

void cancel_reset() {
    set(PM_RSTC, PM_PASSWORD | 0);  // full reset
    set(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}

void PrintHello()
{
    uart_puts("Hello World!\n");
}

void PrintInfo()
{
    // get the board's unique serial number with a mailbox call
    mbox[0] = 7*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = 0x10002;   // get serial number command
    mbox[3] = 4;                    // buffer size
    mbox[4] = 0;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = MBOX_TAG_LAST;

    //mbox[7] = ;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("My board revision is: 0x");
        uart_hex(mbox[5]);
        uart_puts("\n");
    } else {
        uart_puts("Unable to query serial!\n");
    }

    // get the board's unique serial number with a mailbox call
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = 0x10005;   // get serial number command
    mbox[3] = 8;                    // buffer size
    mbox[4] = 0;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;
    mbox[7] = MBOX_TAG_LAST;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("My memory base address is: 0x");
        uart_hex(mbox[5]);
        uart_puts("\n");
        uart_puts("My memory size is: 0x");
        uart_hex(mbox[6]);
        uart_puts("\n");
    } else {
        uart_puts("Unable to query serial!\n");
    }
}

void PrintHelp()
{
    uart_puts("help    : print this help menu\n");
    uart_puts("hello   : print Hello World!\n");
    uart_puts("info    : print system information\n");
    uart_puts("reboot  : reboot the device\n");
}

void main()
{
    // set up serial console
    uart_init();

	PrintHelp();
   	
	char c;
    
    int idx;
    for (idx = 0; idx < size; idx++)
		buffer[idx] = '\0';
    
	// shell
    while(1) {
		idx = 0;
        while(1) {
            c = uart_getc();
			if (c == '\r') {
				uart_send('\n');
				uart_send('\r');
				buffer[idx++] = '\0';
				break;
			}
			uart_send(c);
			buffer[idx++] = c;
        }
        if (buffer[0] == 'h' && buffer[1] == 'e' && buffer[2] == 'l' && buffer[3] == 'p' && buffer[4] == '\0') {
			PrintHelp();
		}
		else if (buffer[0] == 'h' && buffer[1] == 'e' && buffer[2] == 'l' && buffer[3] == 'l' && buffer[4] == 'o' && buffer[5] == '\0') {
			PrintHello();
		}
		else if (buffer[0] == 'r' && buffer[1] == 'e' && buffer[2] == 'b' && buffer[3] == 'o' && buffer[4] == 'o' && buffer[5] == 't' && buffer[6] == '\0') {
			reset(1000);
            //cancel_reset();
		}
        else if (buffer[0] == 'i' && buffer[1] == 'n' && buffer[2] == 'f' && buffer[3] == 'o' && buffer[4] == '\0') {
            PrintInfo();
        }
        else if (buffer[0] == '\0') {
            asm volatile("nop");
        }
		else {
			uart_puts("Command not found\n");
		}

		for (idx = 0; idx < size; idx++)
			buffer[idx] = '\0';
    }
    
}

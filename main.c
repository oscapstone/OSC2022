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
#include "string.h"
#include "power.h"
#include "mbox.h"

void clean_buffer(char * buffer, int buffer_len)
{
    for(int i = 0 ; i < buffer_len ; i++)
        buffer[i] = '\0';
}

void command_help()
{
    uart_puts("\n");
    uart_puts("help\t: print this help menu\n");
    uart_puts("hello\t: print Hello World!\n");
    uart_puts("reboot\t: reboot the device\n");
    uart_puts("mailbox\t: show information through mailbox\n");
}

void command_hello()
{
    uart_puts("\n");
    uart_puts("Hello World!\n");
}

void command_not_found(char * buffer)
{
    uart_puts("\n");
    uart_puts("command *");
    uart_puts(buffer);
    uart_puts("* not exist\n");
}

void command_mailbox()
{
    // get serail number
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = MBOX_TAG_GETSERIAL;   // get serial number command
    mbox[3] = 8;                    // buffer size
    mbox[4] = 8;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;
    uart_puts("\n");
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("serial number is: ");
        uart_hex(mbox[6]);
        uart_hex(mbox[5]);
        uart_puts("\n");
    }
    // get board revision
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = MBOX_TAG_GETBDVS;   // get serial number command
    mbox[3] = 4;                    // buffer size
    mbox[4] = 4;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("board revision is: ");
        uart_hex(mbox[6]);
        uart_hex(mbox[5]);
        uart_puts("\n");
    }

    // get arm memory
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = MBOX_TAG_GETARMMEM;   // get serial number command
    mbox[3] = 8;                    // buffer size
    mbox[4] = 8;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("arm base addr: ");
        uart_hex(mbox[5]);
        uart_puts("\n");
        uart_puts("arm addr size: ");
        uart_hex(mbox[6]);
        uart_puts("\n");
    }
}
void parse_command(char * buffer)
{
    if ( !strcmp(buffer, "help")) command_help();
    else if ( !strcmp(buffer, "hello")) command_hello();
    else if ( !strcmp(buffer, "mailbox")) command_mailbox();
    else if ( !strcmp(buffer, "reboot")) reset();
    else command_not_found(buffer);
}

void main()
{
    // set up serial console
    uart_init();
    
    char buffer[64]={'\0'};
    int buffer_len=0;
    //clean buffer
    clean_buffer(buffer, 64);

    // echo everything back
    while(1) {
        uart_send('#');
        while(1){
            char c = uart_getc();
            uart_send(c);
            if(c=='\n'){
                //parse buffer
                parse_command(buffer);
                //clean buffer
                clean_buffer(buffer, 64);
                buffer_len = 0;
                break;
            }
            buffer[buffer_len++] = c;
        }
    }
}

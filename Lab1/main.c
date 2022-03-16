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
#include "utils.h"
#include "uart.h"
#include "mbox.h"
#include "power.h"
#include "commands.h"

#define NUM_OF_COMMANDS 4
#define CMD_LEN 10
#define MSG_LEN 50
#define BUF_LEN 30


typedef struct commads
{
    char cmd[CMD_LEN];
    char help[MSG_LEN];

} commads;

void read_command(char* buffer){
    uart_puts("\r\n# ");
    int idx=0;
    while(1){
        if(idx >= BUF_LEN) break;
        char c = uart_getc();
        if(c=='\n') {
            uart_puts("\r\n"); // echo
            break;
        }
        else {
            buffer[idx++] = c;
        }
        uart_send(c); // echo
    }
}

void clear_buffer(char* buffer, int size){
    for(int i = 0; i<size; i++){
        buffer[i] = '\0';
    }
}

int compare(char const *a, char const *b){
    //for(int i = 0; i<size; i++){
    while(*a){
        if(*a != *b) return 0;
        if(*a == '\0' && *b == '\0') return 1; /// ????????
        a++; b++;
    }
    return 1; // 
}

void execute_command(char* str, commads cmd_list[NUM_OF_COMMANDS]){

    if(compare(str, "help") == 1){
        for(int i = 0; i<NUM_OF_COMMANDS; i++){
            uart_puts(cmd_list[i].cmd);
            uart_puts("\t: ");
            uart_puts(cmd_list[i].help);
            uart_puts("\r\n");
        }
    }else if(compare(str, "hello") == 1){

        uart_puts("Hello World!");

    }else if(compare(str, "reboot") == 1){

        //uart_puts("reboot system");
        reset();

    }else if(compare(str, "mailbox") == 1){

        // see mailbox detail in https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface

        // get the board's unique serial number with a mailbox call
        mbox[0] = 8*4;                  // length of the message
        mbox[1] = MBOX_REQUEST;         // this is a request message
        
        mbox[2] = MBOX_TAG_GETSERIAL;   // get serial number command
        mbox[3] = 8;                    // buffer size
        mbox[4] = 8;
        mbox[5] = 0;                    // clear output buffer
        mbox[6] = 0;
    
        mbox[7] = MBOX_TAG_LAST;
    
        // send the message to the GPU and receive answer
        if (mbox_call(MBOX_CH_PROP)) {
            uart_puts("My serial number is: ");
            uart_hex(mbox[6]);
            uart_hex(mbox[5]);
            uart_puts("\r\n");
        } else {
            uart_puts("Unable to query serial!\n");
        }
        
        /* ===== get the board's revison ===== */

        mbox[0] = 8*4;                  // length of the message
        mbox[1] = MBOX_REQUEST;         // this is a request message
        
        mbox[2] = MBOX_TAG_BOARD_REVISION;   // get serial number command
        mbox[3] = 4;                    // buffer size
        mbox[4] = 4;
        mbox[5] = 0;                    // clear output buffer
        mbox[6] = 0;
    
        mbox[7] = MBOX_TAG_LAST;
    
        // send the message to the GPU and receive answer
        if (mbox_call(MBOX_CH_PROP)) {
            uart_puts("My board revision is: ");
            uart_hex(mbox[6]);
            uart_hex(mbox[5]);
            uart_puts("\r\n");
        } else {
            uart_puts("Unable to query serial!\n");
        }

        /* ===== get the board's memory info ===== */

        mbox[0] = 8*4;                  // length of the message
        mbox[1] = MBOX_REQUEST;         // this is a request message
        
        mbox[2] = MBOX_TAG_BOARD_MEMORY;   // get serial number command
        mbox[3] = 8;                    // buffer size
        mbox[4] = 8;
        mbox[5] = 0;                    // clear output buffer
        mbox[6] = 0;
    
        mbox[7] = MBOX_TAG_LAST;
    
        // send the message to the GPU and receive answer
        if (mbox_call(MBOX_CH_PROP)) {
            uart_puts("My ARM memory base address in bytes is: ");
            uart_hex(mbox[5]);
            uart_puts("\r\n");
            uart_puts("My ARM memory size in bytes is: ");
            uart_hex(mbox[6]);
            uart_puts("\r\n");
        } else {
            uart_puts("Unable to query serial!\n");
        }


        // echo everything back
        // uart_send(uart_getc());
    }else{
        uart_puts("command not found");
    }
}

void main()
{
    // set up serial console
    uart_init();
    
    // say hello
    uart_puts("Wellcome!\r\n");

    commads cmd_list[NUM_OF_COMMANDS]={
        {.cmd="help", .help="print this help menu"},
        {.cmd="hello", .help="print Hello World!"},
        {.cmd="reboot", .help="reboot the device"},
        {.cmd="mailbox", .help="get mailbox information"},
    };
    
    char input_buffer[BUF_LEN];
    while(1) {
        // echo everything back
        clear_buffer(input_buffer, BUF_LEN);
        // read command
        read_command(input_buffer);
        // uart_puts(input_buffer); // echo input
        execute_command(input_buffer, cmd_list);
    }
}

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
#include "commands.h"

#define NUM_OF_COMMANDS 3
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
        if(*a == '\0') return 1; /// ????????
        a++; b++;
    }
    return 1; // 
}
char compare_c(char const *a, char const *b, int size){
    for(int i = 0; i<size; i++){
        if(a[i] != b[i]) return '0';
        if(a[i] == '\0') return '1'; /// ????????
    }
    return '1'; // 
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

        uart_puts("reboot system");

    }else{
        uart_puts("command not found");
    }
    
}


void main()
{
    // set up serial console
    uart_init();
    
    // say hello
    uart_puts("Hello World!\r\n");

    commads cmd_list[NUM_OF_COMMANDS]={
        {.cmd="help", .help="print this help menu"},
        {.cmd="hello", .help="print Hello World!"},
        {.cmd="reboot", .help="reboot the device"}
    };
    
    char input_buffer[BUF_LEN];
    while(1) {
        // echo everything back
        clear_buffer(input_buffer, BUF_LEN);
        // read command
        read_command(input_buffer);
        // uart_puts(cmd);
        // uart_puts(input_buffer); // echo input

        execute_command(input_buffer, cmd_list);

    }
}

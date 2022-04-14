#include "command.h"
#include "string.h"
#include "shell.h"
#include "uart.h"
#include "mbox.h"

void set(long addr, unsigned int value){
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void reset(int tick){                  // reboot after watchdog timer expire
    set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

void cancel_reset(){
    set(PM_RSTC, PM_PASSWORD | 0);  // full reset
    set(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}

void print_welcome(){
    uart_puts("************    cl3nn0's OS    ************\n");
}

void cmd_err(char *s){
    uart_puts("Err: command ");
    uart_puts(s);
    uart_puts(" not found, try <help>\n");
}

void cmd_help(){
    uart_puts("help      : print this help menu\n");
    uart_puts("hello     : print Hello World!\n");
    uart_puts("revision  : print board revision\n");
    uart_puts("memory    : print ARM memory base address and size\n");
    uart_puts("reboot    : reboot the device\n");
}

void cmd_hello(){
    uart_puts("Hello World!\n");
}

void cmd_revision(){
    volatile unsigned int mbox[36];
    char revision[MAX_BUFFER_LEN];
    if(get_board_revision(mbox)){
        uart_puts("Board Revision: 0x");
        uitohex(mbox[5], revision);
        uart_puts(revision);
        uart_puts("\n");
    }
    else
        uart_puts("Failed to get board revision\n");
}

void cmd_memory(){
    volatile unsigned int mbox[36];
    char addr[MAX_BUFFER_LEN], size[MAX_BUFFER_LEN];
    if(get_arm_memory(mbox)){
        uart_puts("ARM Memory Base Address: 0x");
        uitohex(mbox[5], addr);
        uart_puts(addr);
        uart_puts("\n");
        uart_puts("ARM Memory Size: 0x");
        uitohex(mbox[6], size);
        uart_puts(size);
        uart_puts("\n");
    }
    else
        uart_puts("Failed to get ARM memory base address and size\n");
}

void cmd_reboot(){
    uart_puts("------------     Rebooting     ------------\n");
    reset(100);
    while(1);
}

int readline(char *s, int len){
    unsigned int idx = 0;
    char c;
    while(1){
        c = uart_getc();
        /* check for non-ascii char */
        if(c < 0 || c >= 128)
            continue;
        /* print \r\n */
        if(c == '\n'){
            uart_puts("\n");
            break;
        }
        /* handle the backspace */
        else if(c == '\x7f'){
            if(idx > 0){
                uart_puts("\b");
                uart_puts(" ");
                uart_puts("\b");
                idx--;
            }
            else
                continue;
        }
        /* normal case, print and save the character */
        else{
            uart_send(c);
            if(idx < len)
                s[idx++] = c;
        }
    }
    s[idx] = '\0';
    return idx;
}
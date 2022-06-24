#include "stdlib.h"

uint64_t simple_alloc(size_t size)
{
    uint64_t addr = cur_heap_pos;
    cur_heap_pos += size;
    cur_heap_pos = ALIGN(cur_heap_pos, 16);

    return addr;
}

int atoi(const char *s)
{
    int value, sign;

    /* skip leading while characters */
    while (*s == ' ' || *s == '\t') s++;

    if (*s == '-') sign = -1, s++;
    else sign = 1;

    if (*s >= '0' && *s <= '9') value = (*s - '0');
    else return 0;
    
    s++;
    while (*s != 0)
    {
       if (*s >= '0' && *s <= '9')
       {
           value = value * 10 + (*s - '0');
           s++;
       }
       else return 0;
    }

    return value * sign;
}

void exit(){
    sync_uart_puts("\nexit\n");
    while(1) ;
}

void exit_000() {
    sync_uart_puts("\nexit_000\n");
    while(1) ;
}

void exit_080() {
    sync_uart_puts("\nexit_080\n");
    while(1) ;
}

void exit_100() {
    sync_uart_puts("\nexit_100\n");
    while(1) ;
}

void exit_180() {
    sync_uart_puts("\nexit_180\n");
    while(1) ;
}

void exit_200() {
    sync_uart_puts("\nexit_200\n");
    while(1) ;
}

void exit_280() {
    sync_uart_puts("\nexit_280\n");
    while(1) ;
}

void exit_300() {
    sync_uart_puts("\nexit_300\n");
    while(1) ;
}

void exit_380() {
    sync_uart_puts("\nexit_380\n");
    while(1) ;
}

void exit_400() {
    sync_uart_puts("\nexit_400\n");
    while(1) ;
}

void exit_480() {
    sync_uart_puts("\nexit_480\n");
    while(1) ;
}

void exit_500() {
    sync_uart_puts("\nexit_500\n");
    while(1) ;
}

void exit_580() {
    sync_uart_puts("\nexit_580\n");
    while(1) ;
}
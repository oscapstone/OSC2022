#include "stdlib.h"

static uint32_t heap_start  = 0x10000000;
static uint32_t heap_size = 0x10000000;
static uint32_t cur_heap_pos = 0x10000000;

void *simple_alloc(size_t size)
{
    void *addr = cur_heap_pos;
    cur_heap_pos += size;
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
    uart_puts("\nexit\n");
    while(1) ;
}
#include "malloc.h"
#include "uart.h"

extern char __heap_top;
extern char __heap_start;
extern char __heap_size;

#define SMEM (&__heap_top)
#define EMEM (&( __heap_start)+(unsigned long)&__heap_size)

static char *cur = SMEM;

void *simple_malloc(size_t size)
{
    char *tmp ;
    if ((unsigned long)cur + size > (unsigned long)EMEM) {
        uart_puts("[!] No enough space!\r\n");
        return ;
    }

    tmp = cur;
    cur += size;

    return tmp;
}
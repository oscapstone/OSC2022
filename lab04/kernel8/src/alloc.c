#include "alloc.h"
#include "uart.h"

extern char __heap_size;
extern char __heap_top;
extern char __heap_start;

static char *cur = &__heap_top;

void *simple_malloc(unsigned int size)
{
    // uart_puts("0x");
    // uart_hex((unsigned long)&__heap_size);
    // uart_put_int((unsigned long)&__heap_top);
    // uart_puts("\r\n");
    char *tmp;
    char *heap_end = &__heap_start + (unsigned long)&__heap_size;
    // uart_hex((unsigned long)heap_end);
    // uart_puts("\r\n");
    // uart_hex((unsigned long)cur);
    // uart_puts("\r\n");

    if (((unsigned long)cur + size) > (unsigned long)heap_end) {
        uart_puts("Heap is full!\r\n");
        return 0;
    }

    tmp = cur;
    cur += size;

    return tmp;
}
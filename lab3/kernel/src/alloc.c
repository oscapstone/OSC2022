#include "uart.h"
#include "utils.h"

uint64_t allocate_end = 0x6000000;
void* malloc(int size)
{
    uint64_t location = allocate_end;
    allocate_end += size;
    // uart_puts("allocate memory at:");
    // uart_hex(location);
    // uart_puts(", with size:");
    // uart_hex(size);
    // uart_puts("\r\n");

    return (void *)location;
}
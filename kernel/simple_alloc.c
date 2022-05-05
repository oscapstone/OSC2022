#include "simple_alloc.h"

extern char _heap_start;
static char* ptr = &_simple_alloc_start;

void* simple_alloc(unsigned int size) {
    // point to data
    char* ret = ptr;
    // minimum chunk size = 0x10
    if (size < 0x10)
        size = 0x10;
    // check out of memory
    if ((unsigned long long int)ptr + size > (unsigned long long int)&_simple_alloc_end) {
        uart_printf_async("simple_alloc : No space\r\n");
        return 0;
    }
    // 0x10 alignment
    while(size & 0xf)
        size += 1;
    // move heap ptr
    ptr += size;

    return ret;
}
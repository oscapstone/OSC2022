#include "malloc.h"

extern char _heap_start;
static char* ptr = &_heap_start;

void* malloc(uint32_t size) {
    // point to data, instead of prev_data
    char* ret = ptr + 0x10;
    // minimum chunk size = 0x20
    if (size < 0x18)
        size = 0x18;
    // size + 0x10 - 0x8
    size += 8;
    // 0x10 alignment
    while(size & 0xf)
        size += 1;
    // write chunk size
    *(uint32_t*)(ret - 0x8) = size;
    // move heap ptr
    ptr += size;

    return ret;
}
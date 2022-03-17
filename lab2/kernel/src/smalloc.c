#include "smalloc.h"

extern char _heap_start;
static char* top = &_heap_start;

void* simple_malloc(unsigned long size) {
    char* addr = top;
    top += size;
    return addr;
}
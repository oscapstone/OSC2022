#include "smalloc.h"
#include "uart.h"

void *smalloc(unsigned long size)
{
    if (heap_start + size >= heap_end)
        return NULL;
    unsigned long temp = heap_start;
    heap_start += size;
    return (void *)temp;
}
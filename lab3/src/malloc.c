#include "malloc.h"
extern char _heap_start;
static char* simple_top = &_heap_start;
static char *top = (char*)0x10000000L;
// like C malloc
void *simple_malloc(unsigned int size)
{
    char* r = top+0x10;
    if(size<0x18)size=0x18;  // minimum size 0x20 //like ptmalloc
    size = size + 0x7;
    size = 0x10 + size - size%0x10;
    *(unsigned int*)(r-0x8) = size;
    simple_top += size;
    return r;
}

// like C malloc
void *kmalloc(unsigned int size)
{
    char *r = top + 0x10;
    if (size < 0x18)
        size = 0x18; // minimum size 0x20 //like ptmalloc
    size = size + 0x7;
    size = 0x10 + size - size % 0x10;
    *(unsigned int *)(r - 0x8) = size;
    top += size;
    return r;
}

//TODO
void kfree(void* ptr){

}
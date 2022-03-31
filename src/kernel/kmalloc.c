#include <kmalloc.h>
#include <stdint.h>
#include <stddef.h>

extern uint32_t _heap_start;
extern uint32_t _heap_end;
void *kmalloc_simple_top_chunk;
void *kmalloc_simple_end;

void *kmalloc_simple(size_t size)
{
    if(!kmalloc_simple_top_chunk){
        kmalloc_simple_top_chunk = (void *)&_heap_start;
        kmalloc_simple_end = (void *)&_heap_end;
    }
    //if(size&0xf)size = ((size>>4) + 1) << 4;
    size = (((size-1)>>4)+1)<<4;
    if((uint64_t)kmalloc_simple_top_chunk+size >= (uint64_t)kmalloc_simple_end)
    {
        return 0;
    }
    void *chunk = kmalloc_simple_top_chunk;
    kmalloc_simple_top_chunk = (void *)((uint64_t)kmalloc_simple_top_chunk+size);
    return chunk;
}

void *kmalloc(size_t size)
{
    return kmalloc_simple(size);
}

void kfree(void *ptr)
{
    return ;
}
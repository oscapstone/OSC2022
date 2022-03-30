#pragma once
#include "stdint.h"

//extern uint32_t DTB_ADDR;
extern uint32_t HEAP_START;
static size_t heap_offset=0;

void* simple_malloc(size_t size){
    size_t mem_start = heap_offset + HEAP_START;
    heap_offset+=size;
    return (void*) mem_start;
}

unsigned long __stack_chk_guard;
void __stack_chk_guard_setup(void)
{
     __stack_chk_guard = 0xBAAAAAAD;//provide some magic numbers
}

void __stack_chk_fail(void)                         
{
 /* Error message */                                 
}// will be called when guard variable is corrupted 


void *memcpy(void *dest, const void *src, unsigned int n)
{
    for (unsigned int i = 0; i < n; i++)
    {
        ((char*)dest)[i] = ((char*)src)[i];
    }
}
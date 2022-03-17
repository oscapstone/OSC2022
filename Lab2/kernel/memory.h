#include "stdint.h"

//extern uint32_t DTB_ADDR;
extern uint32_t HEAP_START;
static size_t heap_offset=0;

void* simple_malloc(size_t size){
    size_t mem_start = heap_offset + HEAP_START;
    heap_offset+=size;
    return (void*) mem_start;
}
#include "lib/simple_malloc.h"
#include "mm/mmu.h"

extern int __heap_start;
static struct malloc_state mstate = {
    .last_remainder = (uint8_t*)&__heap_start
};


void* simple_malloc(size_t size){
    void* chunk = mstate.last_remainder;
    size_t nb = req2size(size);
    mstate.last_remainder = mstate.last_remainder + nb;
    return chunk;
}
void* simple_malloc_get_remainder(){
    return mstate.last_remainder;
}

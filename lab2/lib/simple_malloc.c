#include "lib/simple_malloc.h"
#include "debug/debug.h"
#include "types.h"


#define req2size(req) ALIGN_UP(req, 16)
struct malloc_state{
    uint8_t* last_remainder;
}mstate;

void init_malloc_state(void* heap_start){
    INFO("Simple heap start address: %p", heap_start);
    mstate.last_remainder = (uint8_t*)heap_start;
}

void* simple_malloc(size_t size){
    void* chunk = mstate.last_remainder;
    size_t nb = req2size(size);
    mstate.last_remainder = mstate.last_remainder + nb;
    return chunk;
}
void* get_remainder(){
    return mstate.last_remainder;
}

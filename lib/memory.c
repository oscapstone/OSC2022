#include "memory.h"

void* simple_malloc(unsigned int size) {
    void* address = NULL;
    if(heap + size < heap_end) { // should not exceed
        address = (void*)heap;
        while(size--) {
            *(heap++) = 0;
        }
    }

    return address;
}
#include "memory.h"
#include "type.h"

// void* simple_malloc(unsigned int size) {
//     void* address = NULL;
//     if(heap + size < heap_end) { // should not exceed
//         address = (void*)heap;
//         while(size--) {
//             *(heap++) = 0;
//         }
//     }

//     return address;
// }

void* simple_malloc(unsigned int size) {
    char* r = heap + 0x10;
    if(size < 0x18)
        size = 0x18;  // minimum size 0x20 //like ptmalloc
    size = size + 0x7;
    size = 0x10 + size - size % 0x10;
    *(unsigned int*)(r - 0x8) = size;
    heap += size;
    return r;
}
#ifndef _SIMPLE_MALLOC_H_ 
#define _SIMPLE_MALLOC_H_ 

#include "types.h"
#include "debug/debug.h"

#define req2size(req) ALIGN_UP(req, 16)

struct malloc_state{
    uint8_t* last_remainder;
};

extern void* simple_malloc(size_t);
extern void init_malloc_state(void*);
extern void* simple_malloc_get_remainder();
#endif

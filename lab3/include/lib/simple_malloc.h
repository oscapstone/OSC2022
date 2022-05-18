#ifndef _SIMPLE_MALLOC_H_ 
#define _SIMPLE_MALLOC_H_ 

#include "types.h"

extern void* simple_malloc(size_t);
extern void init_malloc_state(void*);
extern void* get_remainder();
#endif

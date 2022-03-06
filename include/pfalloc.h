#ifndef PFALLOC_H
#define PFALLOC_H

#ifndef WITH_STDLIB
#include "type.h"
#else
#include <stdint.h>
#endif

extern uint32_t endkernel;
extern uint32_t endphyspace;


typedef struct {
    uint32_t* addr;
    uint32_t len;
} pageframe_t;



// static uint32_t frame_map[];


pageframe_t* kalloc_frame_init();

pageframe_t* kalloc_frame();

void kfree_frame(pageframe_t*);





#endif
#ifndef HEAP_H
#define HEAP_H


#include "list.h"

#ifndef WITH_STDLIB
#include "type.h"
#else
#include <stdint.h>
#endif



typedef struct {
    uint32_t daddr;
    uint32_t size;
    struct list_head list;
} heap_head_t;


extern volatile struct list_head __FREE_HEAD;
extern volatile struct list_head __ALLOC_HAED;





void _heap_init();

void* hmalloc(size_t size);
void hfree(void* addr);












#endif
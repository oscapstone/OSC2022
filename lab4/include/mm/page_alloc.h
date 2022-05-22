#ifndef _PAGE_ALLOC_H_
#define _PAGE_ALLOC_H_

#include "lib/list.h"
#define BUDDY_GROUP_MEMBER 0x80000000
#define BUDDY_ALLOCATED    0x40000000
#define MAX_ORDER    6 
struct page{
// if (order & BUDDY_MEMBER), then it is freed and it is not a buddy leader
// if (order & BUDDY_ALLOCATED), then it is allocated 
    int32_t order; 
    uint32_t type;
    struct list_head list;
};


#endif


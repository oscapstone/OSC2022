#ifndef _PAGE_ALLOC_H_
#define _PAGE_ALLOC_H_

#include "types.h"
#include "lib/list.h"
#include "lib/print.h"
#include "mm/mm.h"

#define BUDDY_GROUP_MEMBER 0x80000000
#define BUDDY_ALLOCATED    0x40000000
#define MAX_ORDER    6 

#define PAGE_TYPE_RESERVED 1

#define addr_to_pfn(addr) (addr >> PAGE_SHIFT)

struct page{
// if (order & BUDDY_MEMBER), then it is freed and it is not a buddy leader
// if (order & BUDDY_ALLOCATED), then it is allocated 
    int32_t order; 
    uint32_t type;
    struct list_head list;
};

extern void buddy_init(struct list_head*, struct list_head*, struct page*);

#endif


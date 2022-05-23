#ifndef _PAGE_ALLOC_H_
#define _PAGE_ALLOC_H_

#include "types.h"
#include "lib/list.h"
#include "lib/print.h"
#include "lib/bitops.h"
#include "mm/mm.h"

#define BUDDY_GROUP_MEMBER 0x80000000
#define BUDDY_ALLOCATED    0x40000000
#define BUDDY_MAX_ORDER    7 

#define PAGE_TYPE_RESERVED 1
#define PAGE_SHIFT 12
#define PAGE_SIZE (1ul << PAGE_SHIFT)

#define BUDDY_IS_ALLOCATED(x) (((struct page*)x)->order & BUDDY_ALLOCATED)
#define BUDDY_IS_MEMBER(x) (((struct page*)x)->order & BUDDY_GROUP_MEMBER)
#define BUDDY_IS_FREED(x) (!BUDDY_IS_ALLOCATED(x))
#define PAGE_IS_RESERVED(x) (((struct page*)x)->type & PAGE_TYPE_RESERVED)
#define _buddy_ffs(x) ((x) == 0 ? BUDDY_MAX_ORDER : ffs64(x))
#define addr_to_pfn(addr) ((uint64_t)(addr) >> PAGE_SHIFT)
#define pfn_to_addr(n) ((void*)(n << PAGE_SHIFT))
#define page_to_pfn(x) ((uint64_t)((struct page*)x - mem_map))
#define pfn_to_page(x) ((struct page*)&mem_map[x])

struct page{
// if (order & BUDDY_MEMBER), then it is freed and it is not a buddy leader
// if (order & BUDDY_ALLOCATED), then it is allocated 
    uint32_t order; 
    uint32_t type;
    struct list_head list;
};

struct free_list{
    size_t count;
    struct list_head list;
};

struct buddy_system{
    struct free_list free_lists[BUDDY_MAX_ORDER];
};

extern void buddy_init();
extern void* alloc_page();
extern void* alloc_pages(uint32_t);


#endif


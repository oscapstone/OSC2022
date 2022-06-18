
#ifndef __BUDDY_H__
#define __BUDDY_H__

#include "stddef.h"
#include "list.h"
#include "stdlib.h"

#define BUDDY_BASE 0xFFFF000000000000
#define BUDDY_TOP  0xFFFF000030000000
#define PAGE_SIZE  0x1000

#define BUDDY_PAGE_NUM ((BUDDY_TOP - BUDDY_BASE) / PAGE_SIZE)
// #define BUDDY_PAGE_NUM 66

#define PG_RESERVED  3
#define PG_ALLOC 2  // PAGE_ALLOCATED
#define PG_FREE  1  // PAGE_FREE
#define ORDER_VALID  0

#define FREE_LIST_MAX_ORDER 16

#define PAGE2ADDR(id) (id * PAGE_SIZE + BUDDY_BASE)
#define ADDR2PAGE(addr) ((addr - BUDDY_BASE) / PAGE_SIZE)


typedef struct frame {
    uint32_t status; // 0 for the order is valid, 1 for page free, 2 for page allocate.
    uint32_t order;
} frame_t;


// frame_t frame_arr[BUDDY_PAGE_NUM];
frame_t *frame_arr;

struct list_head free_list[FREE_LIST_MAX_ORDER];

void init_buddy();
void build_buddy();
int merge_buddy(uint32_t index, uint32_t order, int log_on);
uint32_t sibling(uint32_t index, uint32_t order);
void reserve_memory(uint64_t start, uint64_t end);
void* buddy_alloc(uint32_t request_order);
void buddy_free(void* addr);
void build_free_list();

#endif
#ifndef __BUDDY__H__
#define __BUDDY__H__
#include "string.h"
#include "list.h"
#include "stddef.h"
#include "uart.h"

// #define MEMORY_START            0x90000
#define MEMORY_START               0x40000


#define PAGE_SHIFT              12
#define PAGE_SIZE               (1 << PAGE_SHIFT)      //4096byte = 4Kb
#define MAX_BUDDY_ORDER         9                      // 2^0 ~ 2^8 => 4k to 1MB
#define MAX_BLOCK_SIZE          (1 << MAX_BUDDY_ORDER) //512
#define MAX_ORDER_SIZE         4096

#define MIN_OBJECT_ORDER        4
#define MAX_OBJECT_ORDER        11
#define MIN_OBJECT_SIZE         (1 << MIN_OBJECT_ORDER)
#define MAX_OBJECT_SIZE         (1 << MAX_OBJECT_ORDER)
#define MAX_ALLOCATOR_NUMBER    MAX_OBJECT_ORDER - MIN_OBJECT_ORDER + 1  //8   16*2^8=2048bytes

struct page
{
    struct list_head list;                // must be put in the front
    int order;
    void *first_free;
    struct dynamic_allocator *allocator;
    int object_count;                     // how many objects this page stores currently

    int page_number;
    int used;
    void *start_address;
};

struct dynamic_allocator
{
    struct page *current_page;
    struct list_head partial;
    struct list_head full;
    int max_object_count;                 // how many objects pages controlled by this allocator can store
    int object_size;
};

void buddy_init();
void dynamic_allocator_init();
void memory_init();

struct page *page_alloc(int order);
void page_free(struct page *block);
void *obj_malloc(int size);
void obj_free(void *object);

void *memory_allocation(int size);
void memory_free(void *address);

int find_buddy(int page_number, int order);

void mm();

void print_buddy_info();

#endif
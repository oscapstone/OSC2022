#ifndef BUDDY_H
#define BUDDY_H
#include "mmu.h"
#include "dtb.h"
#include "list.h"
#include "math.h"
#include "malloc.h"
#include "simple_alloc.h"

#define PAGE_SIZE 0x1000
#define MAX_ORDER 16

extern unsigned int frame_ents_size;
extern unsigned long long int buddy_base;
extern unsigned long long int buddy_end;

typedef struct {
    unsigned int exp;
    unsigned int allocated;
} frame_ent;

typedef struct {
    list_head_t list;
} frame_hdr;

void *idx_to_addr(int idx);
int addr_to_idx(void *ptr);
int num_to_exp(int num);

void mm_init();
void buddy_alloc_init(void *start, void *end);
void page_alloc_init();
void *alloc_pages(int num);
void free_page(void *page);
void memory_reserve(void *start, void *end);

void page_allocator_test();

#endif
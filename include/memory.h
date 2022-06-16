#ifndef MEMORY_H
#define MEMORY_H

#include "list.h"
#include "uart.h"
#include "cpio.h"
#include "address.h"
#include "interrupt.h"
#include "mmu.h"

#define MAXORDER 7
#define MAXCACHEORDER 4 // 32, 64, 128, 256, 512  (for every 32bytes)

// simple_malloc
void *simple_malloc(unsigned int size);
char *memcpy (void *dest, const void *src, unsigned long long len);
void *memset(void *s, int c, size_t n);

#define BUDDYSYSTEM_START  PHY_TO_VIR(0x000000L)
#define BUDDYSYSTEM_PAGE_COUNT 0x3C000 // 0x3C000000 is total size of memory, page size is 0x1000
//buddy system (for >= 4K pages)
void *allocpage(unsigned int size);
void freepage(void *ptr);

void *alloccache(unsigned int size);
void freecache(void *ptr);
void page2caches(int order);

void *malloc(unsigned int size);
void free(void *ptr);

typedef struct frame
{
    struct list_head listhead;
    int val;        // val is order
    int isused;
    int cacheorder; // -1 means isn't used for cache
    unsigned int idx;
} frame_t;

void initMemoryPages();
frame_t *release_redundant(frame_t *frame);
frame_t *get_buddy(frame_t *frame);
frame_t *coalesce(frame_t* f);
void memory_reserve(unsigned long long start, unsigned long long end);
#endif
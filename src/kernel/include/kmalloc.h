#include <stdint.h>
#include <stddef.h>

#ifndef _DEF_KMALLOC
#define _DEF_KMALLOC

void *kmalloc_simple(size_t size);
void *kmalloc(size_t size);
void *kmalloc_(size_t size);
void kfree(void *ptr);
int kmalloc_memory_reserve(uint64_t address, uint64_t size);
void kmalloc_init();
void *buddy_alloc(uint64_t page_num);
void buddy_free(void *ptr);

#define PAGE_SIZE 12

#endif
#include <stdint.h>
#include <stddef.h>

#ifndef _DEF_KMALLOC
#define _DEF_KMALLOC

void *kmalloc_simple(size_t size);
void *kmalloc(size_t size);
void kfree(void *ptr);

#endif
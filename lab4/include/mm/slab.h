#ifndef _SLAB_H_
#define _SLAB_H_

#include "mm/mm.h"
#include "mm/page_alloc.h"

struct slab{
    size_t size; // per object
    void* s_mem; // pointe to the first object
    size_t inuse;
    void* free; // free list
    struct list_head list;
};

#define KMEM_MIN_SIZE 32
#define KMEM_MAX_SIZE (PAGE_SIZE - ALIGN_UP(sizeof(struct slab), KMEM_MIN_SIZE))


extern struct slab* slab_create(size_t obj_size);
extern void slab_destroy(struct slab* slab);
extern void *slab_alloc(struct slab);
extern void slab_free(struct slab* slab, void* obj);

void* kmalloc(size_t size);
void kfree(void* obj);
#endif

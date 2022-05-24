#ifndef _SLAB_H_
#define _SLAB_H_

#include "mm/mm.h"
#include "mm/page_alloc.h"
#include "lib/list.h"
#include "lib/string.h"
#include "debug/debug.h"

struct slab{
    size_t size; // object size
    void* s_mem; // pointe to the first object
    size_t inuse;
    struct list_head free_list; // free list
    struct list_head list; // next slab
};

#define SLAB_ALIGNMENT (ALIGN_UP(sizeof(struct list_head), 0x20))
#define SLAB_SIZE (ALIGN_UP(sizeof(struct slab), SLAB_ALIGNMENT))
#define SLAB_MAX_OBJECT_SIZE (PAGE_SIZE * (1 << (BUDDY_MAX_ORDER - 1)) - SLAB_SIZE)

#define KMEM_MIN_SIZE SLAB_ALIGNMENT 
#define KMEM_ALIGNMENT SLAB_ALIGNMENT 
#define KMEM_CACHE_NUM (SLAB_MAX_OBJECT_SIZE / KMEM_ALIGNMENT + 1)

extern struct slab* slab_create(size_t);
extern void slab_destroy(struct slab*);
extern void *slab_alloc(struct slab*);
extern void slab_free(struct slab* , void*);

extern void* kmalloc(size_t);
extern void kfree(void*);
extern void kmalloc_init();

extern void debug_kmalloc();
extern void debug_slab();
#endif

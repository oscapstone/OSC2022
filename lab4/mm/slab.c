#include "mm/slab.h"

// 32, 64, 96, ...,  4096
struct slab* kmem_cache[ PAGE_SIZE / KEM_MIN_SIZE + 1] = {0};
struct slab* kmem_large;



struct slab* slab_create(size_t obj_size){

}
void slab_destroy(struct slab* slab){
}

void *slab_alloc(struct slab){
}

void slab_free(struct slab* slab, void* obj){

}

void* kmalloc(size_t size){

}

void kfree(void* obj){
}

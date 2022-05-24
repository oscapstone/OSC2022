#include "mm/slab.h"

struct list_head kmem_cache[KMEM_CACHE_NUM] = {0};

struct slab* slab_create(size_t obj_size){
    size_t allocated_size; 
    uint32_t order = 0;
    size_t page_num;
    void* pages;
    struct slab * pslab;
    
    // object size align to slab_alignment
    obj_size = ALIGN_UP(obj_size, SLAB_ALIGNMENT);

    // check if obj_size exceed slab's limit
    if(obj_size > SLAB_MAX_OBJECT_SIZE){
        LOG("invalid obj_size: %l, slab_max_object_size is %l", obj_size, SLAB_MAX_OBJECT_SIZE);
        return NULL;
    }

    // get sufficient order
    page_num = ALIGN_UP(obj_size + SLAB_SIZE, 4096) >> PAGE_SHIFT;
    order = BUDDY_MAX_ORDER - 1;
    pages = alloc_pages(order);
    if(pages == NULL){
        LOG("Not enough page for slab");
        return NULL;
    }
    
    // initialize struct slab
    pslab = pages;
    pslab->size = obj_size;
    pslab->s_mem = pages + SLAB_SIZE;
    pslab->inuse = 0;
    INIT_LIST_HEAD(&pslab->free_list);

    return pslab;
}

void slab_destroy(struct slab* slab){
    size_t page_num; 
    struct page* page = pfn_to_page(addr_to_pfn(slab));
    
    LOG("slab_destroy recycle order %u buddy %p", (void*)slab, get_page_order(page));
    // free pages
    free_pages((void*)slab, get_page_order(page));
}

void *slab_alloc(struct slab* slab){
    void *ret = NULL;
    struct page* page;
    uint32_t order;
    size_t max_size;
    if(!list_empty(&slab->free_list)){
        // check free list  
        ret = slab->free_list.next; 
        list_del(slab->free_list.next);
        slab->inuse++; 
    }else{
        page = pfn_to_page(addr_to_pfn(slab));
        order = get_page_order(page); 
        max_size = (1 << order) * PAGE_SIZE;
        if(SLAB_SIZE + (slab->inuse + 1) * slab->size > max_size){
            ret = NULL;
        }else{
            ret = (uint8_t*)slab->s_mem + slab->inuse * slab->size;
            slab->inuse++;
        }
    }
    return ret;
}

void slab_free(struct slab* slab, void* obj){
    list_add((struct list_head*)obj, &slab->free_list);
    slab->inuse--;
}

void kmalloc_init(){
    for(uint64_t i = 0 ; i < KMEM_CACHE_NUM ; i++){
        INIT_LIST_HEAD(&kmem_cache[i]);
    }
}
void* kmalloc(size_t size){
    uint64_t cache_idx;
    struct list_head* cache_list, *node;
    struct slab *s;
    void * ret = NULL;

    size = ALIGN_UP(size, SLAB_ALIGNMENT);

    LOG("kmalloc(%u) start", size);
    if(size > SLAB_MAX_OBJECT_SIZE){
        LOG("kmalloc invalid size %l", size); 
        return NULL;
    }
    
    // allocate memory from corresponding cache bin 
    cache_idx = ALIGN_UP(size, SLAB_ALIGNMENT) / SLAB_ALIGNMENT;
    cache_list = &kmem_cache[cache_idx];

    list_for_each(node, cache_list){
        s = list_entry(node, struct slab, list);
        ret = slab_alloc(s);
        if(ret != NULL){
            LOG("kmalloc(%u) found a free object %p in kmem_cache[%u]", size, ret, cache_idx);
            break;
        }
    } 
    
    if(ret == NULL){
        s = slab_create(size);
        list_add(&s->list, cache_list);
        ret = slab_alloc(s);
        if(ret != NULL){
            LOG("kmalloc(%u) found a free object %p in kmem_cache[%u]", size, ret, cache_idx);
        }
    }
        
    
    LOG("kmalloc s->inuse: %x", s->inuse);
    LOG("kmalloc(%u) end, ret: %p", size, ret);
    return ret;
}

void kfree(void* obj){
    uint64_t pfn = addr_to_pfn(ALIGN_DOWN(obj, PAGE_SIZE));
    struct page* page = pfn_to_page(pfn);
    struct page* buddy_leader = get_buddy_leader(page);
    struct slab* s = pfn_to_addr(page_to_pfn(buddy_leader));

    slab_free(s, obj);
    
    if((int64_t)s->inuse < 0){
        LOG("[Error]: s->inuse : %p", s->inuse);
    }
    if(s->inuse == 0){
        LOG("kfree(%p) triger recycle unused slab", obj);
        list_del(&s->list);
        slab_destroy(s);
    }
}

#define TEST_SIZE 130 
#define TEST_INVALID_SIZE ((1 << (BUDDY_MAX_ORDER - 1)) * PAGE_SIZE - SLAB_SIZE + 1)
void debug_slab(){
    uint32_t i, max_i;
    uint8_t *a[ (1 << BUDDY_MAX_ORDER - 1) * PAGE_SIZE / TEST_SIZE];

    struct slab* s = slab_create(TEST_SIZE);
    
    for(i = 0 ; i < 1000; i++){
        a[i] = slab_alloc(s);
        LOG("%u: slab_alloc(%u) = %p", i, TEST_SIZE, a[i]);
        
        if(a[i] == NULL){
            LOG("slab is full");
            break;
        }
    }
    max_i = i; 
    for(i = 0 ; i < max_i ; i++){
        slab_free(s, a[i]);
    }

    for(i = 0 ; i < 1000; i++){
        a[i] = slab_alloc(s);
        LOG("%u: slab_alloc(%u) = %p", i, TEST_SIZE, a[i]);

        if(a[i] == NULL){
            LOG("slab is full");
            break;
        }
    }



    for(i = 0 ; i < max_i ; i++){
        slab_free(s, a[i]);
    }

    slab_destroy(s);
    uint64_t pfn = addr_to_pfn(s);
    struct page* page = pfn_to_page(pfn);

    if(BUDDY_IS_FREED(page) && get_page_order(page) == BUDDY_MAX_ORDER - 1){
        LOG("slab_destroy success");
    }else{

        LOG("slab_destroy failed");
    }

    s = slab_create(TEST_INVALID_SIZE);

    if(s == NULL){
        LOG("create super large slab %l failed", TEST_INVALID_SIZE);
    }else{
        LOG("create super large slab %l success", TEST_INVALID_SIZE);
    }
}

void debug_kmalloc(){
    uint32_t i, max_i;
    uint8_t *a[20];

    
    for(i = 0 ; i < 20; i++){
        a[i] = kmalloc(TEST_SIZE);
        LOG("%l: kmalloc(%u) = %p", i, TEST_SIZE, a[i]);
        
        if(a[i] == NULL){
            LOG("kmalloc is failed");
            break;
        }
    }

    max_i = i; 
    for(i = 0 ; i < max_i ; i++){
        LOG("kfree(%p)", a[i]);
        kfree(a[i]);
    }
/*
    for(i = 0 ; i < 200; i++){
        a[i] = kmalloc(TEST_SIZE);
        LOG("%u: kmalloc(%u) = %p", i, TEST_SIZE, a[i]);
        
        if(a[i] == NULL){
            LOG("kmalloc is failed");
            break;
        }
    }

    for(i = 0 ; i < max_i ; i++){
        kfree(a[i]);
    }
*/
}

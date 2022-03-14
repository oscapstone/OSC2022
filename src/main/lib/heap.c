#include "heap.h"



volatile struct list_head __FREE_HEAD;
volatile struct list_head __ALLOC_HAED;


extern uint32_t _heap_start;
extern uint32_t _heap_end;

void _heap_init() {

    INIT_LIST_HEAD(&__FREE_HEAD);
    INIT_LIST_HEAD(&__ALLOC_HAED);

    heap_head_t* hptr = &_heap_start;

    hptr->daddr = (uint32_t)hptr + sizeof(heap_head_t);
    hptr->size = _heap_end - (hptr->daddr) - sizeof(heap_head_t);
    INIT_LIST_HEAD(&hptr->list);

    list_add_tail(&hptr->list, &__FREE_HEAD);

}


void* hmalloc(size_t size) {

    heap_head_t *item, *safe;

    heap_head_t* alloc = NULL;

    list_for_each_entry_safe(item, safe, (&__FREE_HEAD), list)
    {
        if(item->size >= size) { //* first best fit
            alloc = item->daddr;
            alloc->daddr = (char*)alloc + sizeof(heap_head_t);
            INIT_LIST_HEAD(&alloc->list);

            if(item->size - size == 0) {
                list_del_init(&item->list);
            } else {
                item->size -= size;
                item->daddr = alloc->daddr + size;
                
            }
            //* allocate to ALLOC_HEAD
            list_add(&alloc->list, &__ALLOC_HAED);
            break;
        }
    }

    if(alloc == NULL)
        return NULL;


    return ((void*)alloc->daddr);
}

void hfree(void* addr) {


    heap_head_t *item, *safe;

    list_for_each_entry_safe(item, safe, &__ALLOC_HAED, list) {
        if(item->daddr == addr) {
            list_del_init(&item->list);
            list_add_tail(&item->list, &__FREE_HEAD);
        }
    }


}


#include "mm/page_alloc.h"

void _init_mem_map(struct list_head* rsvmap, struct list_head* unusedmap, struct page* mem_map){
    printf("size: %d, mem_map: %p\r\n",sizeof(struct page), mem_map);
    struct list_head *node;
    struct mem_block *mb;
    uint64_t start_pfn, end_pfn, size;
    list_for_each(node, unusedmap){
        mb = list_entry(node, struct mem_block, list);

        start_pfn = addr_to_pfn(mb->start); 
        end_pfn = addr_to_pfn(mb->end);
        size = (end_pfn - start_pfn) * sizeof(struct page);

        memset(&mem_map[start_pfn], '\0', size);
    }

    list_for_each(node, rsvmap){
        mb = list_entry(node, struct mem_block, list);

        start_pfn = addr_to_pfn(mb->start); 
        end_pfn = addr_to_pfn(mb->end);
        for(uint64_t i = start_pfn ; i < end_pfn ; i++){
            memset(&mem_map[i], '\0', sizeof(struct page));
            mem_map[i].type |= PAGE_TYPE_RESERVED;
        }
    }
}
void buddy_init(struct list_head* rsvmap, struct list_head* unusedmap, struct page* mem_map){
    INFO("Initialize buddy system...");
    _init_mem_map(rsvmap, unusedmap, mem_map);
    
}

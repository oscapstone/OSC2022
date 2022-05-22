#include "mm/page_alloc.h"

static struct buddy_system buddy;

void _init_mem_map(){
    struct list_head *node;
    struct mem_block *mb;
    uint64_t start_pfn, end_pfn, size;
    list_for_each(node, &mem_unusedmap){
        mb = list_entry(node, struct mem_block, list);

        start_pfn = addr_to_pfn(mb->start); 
        end_pfn = addr_to_pfn(mb->end);
        size = (end_pfn - start_pfn) * sizeof(struct page);

        memset(&mem_map[start_pfn], '\0', size);
    }

    list_for_each(node, &mem_rsvmap){
        mb = list_entry(node, struct mem_block, list);

        start_pfn = addr_to_pfn(mb->start); 
        end_pfn = addr_to_pfn(mb->end);
        for(uint64_t i = start_pfn ; i < end_pfn ; i++){
            memset(&mem_map[i], '\0', sizeof(struct page));
            mem_map[i].type |= PAGE_TYPE_RESERVED;
        }
    }
}

void _free_pages(uint64_t pfn, uint32_t order){
    uint64_t end = pfn + (1 << order);
    // initialize buddy group leader and add it to free list
    mem_map[pfn].order = order;
    list_add(&mem_map[pfn].list, &buddy.free_lists[order].list);
    buddy.free_lists[order].count++;

    // initialize buddy group members

    for(uint32_t i = pfn + 1 ; i < end ; i++){
        mem_map[pfn].order |= BUDDY_GROUP_MEMBER;
    }
}

void _free_pages_memory(uint64_t start, uint64_t end){
    // start and end is page frame number ( pfn )
    uint64_t order;
    while(start < end){
        order = min(BUDDY_MAX_ORDER - 1, _buddy_ffs(start));

        // decrease the order if last page's pfn in buddy is larger than end 
        while(start + (1 << order) > end) order--;

        _free_pages(start, order);
        start += (1 << order);
    }
}

void print_buddy_statistics(){
    uint64_t free_page_count = 0;
    uint64_t tmp;
    for(uint32_t i = 0 ; i < BUDDY_MAX_ORDER ; i++){
        tmp = buddy.free_lists[i].count;
        INFO("free page's count of order %l = %l", i, tmp);
        free_page_count += tmp * (1 << i);
    }
    INFO("Total free page #: %l", free_page_count);
}

void buddy_init(){
    struct list_head * node;
    struct mem_block* mb;
    uint64_t start_pfn, end_pfn;

    INFO("Initialize buddy system...");
    // Initialize global variable 
    memset(&buddy, '\0', sizeof(struct buddy_system));
    for(uint32_t i = 0 ; i < BUDDY_MAX_ORDER; i++){
        INIT_LIST_HEAD(&buddy.free_lists[i].list);
    }

    // Initialize mem map
    _init_mem_map();

    // Add free page frame to buddy system
    list_for_each(node, &mem_unusedmap){
        mb = list_entry(node, struct mem_block, list);
        start_pfn = addr_to_pfn(mb->start);
        end_pfn = addr_to_pfn(mb->end);

        _free_pages_memory(start_pfn, end_pfn);
    }
    
    print_buddy_statistics();
}

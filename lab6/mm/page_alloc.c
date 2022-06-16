#include "mm/page_alloc.h"
static struct buddy_system buddy;

void _init_mem_map(){
    struct list_head *node;
    struct mem_block *mb;
    uint64_t start_pfn, end_pfn, size;
    list_for_each(node, &mem_unusedmap){
        mb = list_entry(node, struct mem_block, list);

        start_pfn = virt_to_pfn(mb->start + UPPER_ADDR_SPACE_BASE); 
        end_pfn = virt_to_pfn(mb->end + UPPER_ADDR_SPACE_BASE);
        size = (end_pfn - start_pfn) * sizeof(struct page);

        memset(&mem_map[start_pfn], '\0', size);
    }

    list_for_each(node, &mem_rsvmap){
        mb = list_entry(node, struct mem_block, list);

        start_pfn = virt_to_pfn(mb->start + UPPER_ADDR_SPACE_BASE); 
        end_pfn = virt_to_pfn(mb->end + UPPER_ADDR_SPACE_BASE);
        for(uint64_t i = start_pfn ; i < end_pfn ; i++){
            memset(&mem_map[i], '\0', sizeof(struct page));
            mem_map[i].type |= PAGE_TYPE_RESERVED;
        }
    }
}

void __free_pages(struct page* page, uint32_t order){
    volatile uint64_t daif;
    struct page* end_page = page + (1 << order);

    // initialize buddy group leader and add it to free list
    page->order = order;
    list_add(&page->list, &buddy.free_lists[order].list);
    buddy.free_lists[order].count++;

    // initialize buddy group members

    for(struct page* p = page + 1 ; p < end_page ; p++){
        p->order = BUDDY_GROUP_MEMBER;
    }
}

void _free_pages_memory(uint64_t start, uint64_t end){
    // start and end is page frame number ( pfn )
    uint64_t order;
    while(start < end){
        order = min(BUDDY_MAX_ORDER - 1, _buddy_ffs(start));

        // decrease the order if last page's pfn in buddy is larger than end 
        while(start + (1 << order) > end) order--;

        __free_pages(&mem_map[start], order);
        start += (1 << order);
    }
}

void _free_pages(struct page* page, uint32_t order){
    struct page* buddy_page; 
    uint64_t pfn = page_to_pfn(page);
    uint64_t buddy_pfn;

    // check the order of page
    if(BUDDY_IS_FREED(page)){
        INFO("Error: _free_pages(%u) try to free freed pfn %p", order, pfn);
        return;
    }
    if(order > _buddy_ffs(pfn)){
        INFO("Error: _free_pages(%u) try to free pfn %p", order, pfn);
        return;
    }
    
    while(order < BUDDY_MAX_ORDER - 1){
        buddy_pfn = find_buddy_pfn(pfn, order); 
        buddy_page = pfn_to_page(buddy_pfn); 

        if(!PAGE_IS_RESERVED(buddy_page) && BUDDY_IS_FREED(buddy_page) && buddy_page->order == order){
            // Check if buddy is not reserved and freed
            LOG("%p is buddy of %p in order %u free list and it can be merged", pfn_to_virt(buddy_pfn), pfn_to_virt(pfn), order);
            list_del(&buddy_page->list);
            buddy.free_lists[order].count--;
        }else{
            break;
        }

        pfn = pfn & ~(1 << order);
        page = pfn_to_page(pfn);
        order++;
    }

    LOG("Merge %p into order %u free list ", pfn_to_virt(pfn), order);
    __free_pages(page, order);
    LOG("end merge pages");
}

// free 2^order pages
void free_pages(void* addr, uint32_t order){
    LOG("_free_pages(%p, %u)",addr, order);
    volatile uint64_t daif;
    uint64_t pfn = virt_to_pfn(addr);
    struct page *page = pfn_to_page(pfn);
    
    daif = local_irq_disable_save();
    _free_pages(page, order);
    local_irq_restore(daif);
}

// free one page
void free_page(void* addr){
    volatile uint64_t daif;
    uint64_t pfn = virt_to_pfn(addr);
    struct page *page = pfn_to_page(pfn);

    daif = local_irq_disable_save();
    _free_pages(page, 0);
    local_irq_restore(daif);
}

// split large buddy group to two small buddy groups
void expand(struct page *page, uint32_t high, uint32_t low){
    struct page* tmp_page;
    volatile uint64_t daif;
    while(high > low){
        high--;
        tmp_page = page + (1 << high);
       
        LOG("add page %p to order %u free list",page_to_virt(tmp_page) , high);
        __free_pages(tmp_page, high);
    }
}

struct page* _alloc_pages(uint32_t order){
    LOG("_alloc_pages(%u)", order);
    struct list_head *node;
    struct free_list* free_list;
    struct page* page, *tmp_page;
   
    // find appropriate buddy group for user
    for(uint32_t i = order ; i < BUDDY_MAX_ORDER ; i++){
        free_list = &buddy.free_lists[i];

        if(list_empty(&free_list->list)){
            LOG("free list of order %u is empty", i);
            continue;
        }
       
        page = list_first_entry(&free_list->list, struct page, list); 
        list_del(&page->list);
        free_list->count--;
        LOG("get free page %p from order %u", page_to_virt(page), i); 
        

        for(uint32_t j = 0 ; j < (1 << order) ; j++){
            page[j].order = BUDDY_ALLOCATED|order;
            page[j].buddy_leader = page;
        }

        if(i > order){
            LOG("start expand pages of order %u to order %u", i, order); 
            expand(page, i, order);
            LOG("end expand pages from order %u to order %u", i, order); 
        }
        return page; 
    }
    return NULL;
}

// return 2^order pages
void* alloc_pages(uint32_t order){
    if(order > BUDDY_MAX_ORDER - 1) return NULL;
    volatile uint64_t daif;

    daif = local_irq_disable_save();
    struct page* page = _alloc_pages(order);
    local_irq_restore(daif);
    uint64_t pfn = page_to_pfn(page);

    return pfn_to_virt(pfn);
}

// return one page
void* alloc_page(){
    volatile uint64_t daif;
    daif = local_irq_disable_save();
    struct page* page = _alloc_pages(0);
    local_irq_restore(daif);

    uint64_t pfn = page_to_pfn(page);
    return pfn_to_virt(pfn);
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

uint8_t* _debug_alloc_page(uint32_t order){
    LOG("###########################################");
    uint64_t free_page_count = 0;
    uint64_t tmp;
    uint8_t * ret;
    for(uint32_t i = 0 ; i < BUDDY_MAX_ORDER ; i++){
        tmp = buddy.free_lists[i].count;
        LOG("free page's count of order %l = %l", i, tmp);
        free_page_count += tmp * (1 << i);
    }
    ret = alloc_pages(order);
    LOG("Total free page #: %l", free_page_count);
    LOG("allocate page: %p", ret);
    return ret;
}

uint8_t* _debug_free_page(void *addr, uint32_t order){
    LOG("###########################################");
    uint64_t free_page_count = 0;
    uint64_t tmp;
    uint8_t * ret;
    for(uint32_t i = 0 ; i < BUDDY_MAX_ORDER ; i++){
        tmp = buddy.free_lists[i].count;
        LOG("free page's count of order %l = %l", i, tmp);
        free_page_count += tmp * (1 << i);
    }
    free_pages(addr, order);
    LOG("Total free page #: %l", free_page_count);
    LOG("freed page: %p", addr);
    return ret;
}

void debug_buddy(){
    uint8_t *arr[1500];
    int32_t i = 0, count = 0;
    struct list_head* node;
    struct page* page, *target_page;
    for(i = 0 ; i < 3; i++){
        arr[count] = _debug_alloc_page(0);
        count++;
    }
    for(i = 0 ; i < 3; i++){
        arr[count] = _debug_alloc_page(1);
        count++;
    }
    for(i = 0 ; i < 4; i++){
        arr[count] = _debug_alloc_page(2);
        count++;
    }
    for(i = 0 ; i < 1; i++){
        arr[count] = _debug_alloc_page(0);
        count++;
    }
    for(i = 0 ; i < 3; i++){
        arr[count] = _debug_alloc_page(3);
        count++;
    }
    for(i = 0 ; i < 1; i++){
        arr[count] = _debug_alloc_page(0);
        count++;
    }
    for(i = 0 ; i < 1; i++){
        arr[count] = _debug_alloc_page(1);
        count++;
    }
    for(i = 0 ; i < 1; i++){
        arr[count] = _debug_alloc_page(2);
        count++;
    }
    for(i = 0 ; i < 1400; i++){
        arr[count] = _debug_alloc_page(0);
        count++;
    }
    LOG("###########################################");
    for(i = 0 ; i < count ; i++){
        target_page = virt_to_page(arr[i]);
        if(!BUDDY_IS_ALLOCATED(target_page)){
            LOG("******* page %p didn't set to allocated *******", page_to_virt(page));
            goto error;
        }
        for(uint32_t j = 0 ; j < BUDDY_MAX_ORDER ; j++){
            list_for_each(node, &buddy.free_lists[j].list){
                page = list_entry(node, struct page, list);
                if(target_page == page){
                    LOG("******* allocated page didn't remove from free list *******", page_to_virt(page));
                    goto error;
                }
            }
        }
    }
    count = 0;
    for(i = 0 ; i < 3; i++){
        _debug_free_page(arr[count], 0);
        count++;
    }
    for(i = 0 ; i < 3; i++){
        _debug_free_page(arr[count], 1);
        count++;
    }
    for(i = 0 ; i < 4; i++){
        _debug_free_page(arr[count], 2);
        count++;
    }
    for(i = 0 ; i < 1; i++){
        _debug_free_page(arr[count], 0);
        count++;
    }
    for(i = 0 ; i < 3; i++){
        _debug_free_page(arr[count], 3);
        count++;
    }
    for(i = 0 ; i < 1; i++){
        _debug_free_page(arr[count], 0);
        count++;
    }
    for(i = 0 ; i < 1; i++){
        _debug_free_page(arr[count], 1);
        count++;
    }
    for(i = 0 ; i < 1; i++){
        _debug_free_page(arr[count], 2);
        count++;
    }
    for(i = 0 ; i < 1400; i++){
        _debug_free_page(arr[count], 0);
        count++;
    }
    // check the integrity of linked list by using grep to check how many unique lines
    //
    // ./debug.sh > t
    // grep  "\[test\]" t|sort|uniq|wc -l
    //
    struct list_head* tmp_node;
    struct page* tmp_page;
    for(uint32_t i = 0 ; i < BUDDY_MAX_ORDER ; i++){
        list_for_each(tmp_node, &buddy.free_lists[i].list){
            tmp_page = list_entry(tmp_node, struct page, list); 
            for(uint32_t j = 0; j < (1 << i) ; j++){
                printf("[test] : %p\n", page_to_virt(tmp_page + j));
            }
        }
    }
    LOG("******* Pass the testcases *******");
    return; 
error:
    LOG("******* Something is wrong in buddy system alloc_pages *******");
    return;
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
        start_pfn = virt_to_pfn(mb->start + UPPER_ADDR_SPACE_BASE);
        end_pfn = virt_to_pfn(mb->end + UPPER_ADDR_SPACE_BASE);

        _free_pages_memory(start_pfn, end_pfn);
    }
    
}

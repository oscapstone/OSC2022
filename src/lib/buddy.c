#include "buddy.h"

struct page page_list[MAX_ORDER_SIZE];                         //total 4096*4KB
struct list_head free_buddy_list[MAX_BUDDY_ORDER + 1];         // freelist 2^0*4KB ~ 2^9*4KB = 4KB ~ 2048KB
struct dynamic_allocator allocator_pool[MAX_ALLOCATOR_NUMBER]; //object_pool 16B ~ 2048B

void buddy_init()
{
    // Initial memory page
    for (int i = 0; i < MAX_ORDER_SIZE; i++)
    {
        page_list[i].order = -1; // order = -1 if this page is available but it belongs to larger contiguous memory block
        page_list[i].page_number = i;
        page_list[i].used = 0;
        page_list[i].start_address = (void *)MEMORY_START + i * PAGE_SIZE;
        page_list[i].allocator = NULL;
    }

    // Initial free_buddy_list
    for (int i = 0; i < MAX_BUDDY_ORDER + 1; i++)
    {
        list_head_init(&(free_buddy_list[i]));
    }

    // the address of page_list[i].list is equal to the address of page_list[i]
    // thus we can get the address of the page from the list node
    for (int i = 0; i < MAX_ORDER_SIZE; i += MAX_BLOCK_SIZE)
    {
        page_list[i].order = MAX_BUDDY_ORDER;
        list_insert_prev(&page_list[i].list, &free_buddy_list[MAX_BUDDY_ORDER]);
    }
}

void dynamic_allocator_init()
{
    struct dynamic_allocator *allocator;

    for (int i = MIN_OBJECT_ORDER; i < MAX_OBJECT_ORDER + 1; i++) 
    {
        allocator = &allocator_pool[i - MIN_OBJECT_ORDER]; 

        allocator->current_page = NULL;
        allocator->object_size = (1 << i); // 2^4 ~ 2^11
        allocator->max_object_count = PAGE_SIZE / (1 << i); // 2^8 ~ 2^1

        list_head_init(&allocator->full);
        list_head_init(&allocator->partial);
    }
}

void memory_init()
{
    buddy_init();
    dynamic_allocator_init();
}

struct page *page_alloc(int order)
{
    if ((order > MAX_BUDDY_ORDER) || (order < 0))
    {
        return 0;
    }

    for (int cur_order = order; cur_order < MAX_BUDDY_ORDER + 1; cur_order++)
    {
        // go to next order if free buddy list has no this order now
        if (list_empty(&free_buddy_list[cur_order]))
        {
            continue;
        }

        // free block found
        struct page *tmp_block = (struct page *)free_buddy_list[cur_order].next; // get first item in buddy
        list_remove(&tmp_block->list, &tmp_block->list);// remove from free body list

        tmp_block->used = 1;
        tmp_block->order = order;

        // find the minimum order to allocate
        while (cur_order > order) 
        {
            cur_order--;

            // transform the bigger order to little order till the cur_order = order
            // thus, we can use the minimum order to allocate  
            int bottom_page_number = find_buddy(tmp_block->page_number, cur_order); 
            struct page *bottom = &page_list[bottom_page_number];
            bottom->order = cur_order;

            list_insert_prev(&bottom->list, &free_buddy_list[cur_order]); // release to free_body_list

            uart_puts("bottom->page_number:");
            uart_hex(bottom->page_number);
            uart_puts("\t");
            uart_puts("bottom->order:");
            uart_hex(bottom->order);
            uart_puts("\r\n");

        }
        uart_puts("page_number:");
        uart_hex(tmp_block->page_number);
        uart_puts("\t");
        uart_puts("tmp_block->order:");
        uart_hex(tmp_block->order);
        uart_puts("\r\n");

        uart_puts("[page_alloc] done\r\n");
        return tmp_block;
    }
    uart_puts("[page_alloc] no free space!\r\n");

    return 0;
}

void page_free(struct page *block)
{
    struct page *buddy, *left, *right;
    int buddy_page_number;

    if (block->used == 0)
    {
        uart_puts("[page_free] this block is already freed! \r\n");
        return;
    }
    uart_puts("block->page_number:");
    uart_hex(block->page_number);
    uart_puts("\t");
    uart_puts("block->order:");
    uart_hex(block->order);
    uart_puts("\r\n");

    block->used = 0;
    // clean the point to the allocator 
    block->allocator = NULL;

    buddy_page_number = find_buddy(block->page_number, block->order);
    buddy = &page_list[buddy_page_number]; // find where the buddy of the given page

    // iterate if buddy can be merged
    while (buddy->order == block->order && buddy->order < MAX_BUDDY_ORDER && !buddy->used)
    {
        list_remove(&buddy->list, &buddy->list); // remove from free body list

        if (buddy->page_number > block->page_number)
        {
            left = block;
            right = buddy;
        }
        else
        {
            left = buddy;
            right = block;
        }
        uart_puts("left->page_number:");
        uart_hex(left->page_number);
        uart_puts("\t");
        uart_puts("left->order:");
        uart_hex(left->order);
        uart_puts("\t");
        uart_puts("right->page_number:");
        uart_hex(right->page_number);
        uart_puts("\t");
        uart_puts("right->order:");
        uart_hex(right->order);
        uart_puts("\r\n");

        right->order = -1;
        left->order++;

        // next iteration
        block = left;
        buddy_page_number = find_buddy(block->page_number, block->order);
        buddy = &page_list[buddy_page_number];
    }

    // stop merge
    list_insert_prev(&block->list, &free_buddy_list[block->order]);
    uart_puts("[page_free] done\n\n");
}

void *obj_malloc(int token)
{
    struct dynamic_allocator *allocator = &allocator_pool[token];

    if (allocator->current_page == NULL) 
    {
        struct page *temp_page;

        // fill up pages with free space first
        // partial not empty, there are some page can be used
        if (!list_empty(&allocator->partial))
        {
            temp_page = (struct page *)allocator->partial.next; // get first item in buddy
            list_remove(&temp_page->list, &temp_page->list); 
        }
      
        // page not enough, request a new page from buddy system
        else
        {
            temp_page = page_alloc(0);

            temp_page->allocator = allocator;
            temp_page->object_count = 0;

            // first_free points to the starting address of the page when allocated
            temp_page->first_free = temp_page->start_address;
            
            // block i saves the offset number for block i+1
            for (int i = 0; i < allocator->max_object_count; i++)
            {
                *(int *)(temp_page->start_address + i * allocator->object_size) = (i + 1) * allocator->object_size;
            }
        }

        allocator->current_page = temp_page;
    }

    struct page *current_page = allocator->current_page;
    void *object = current_page->first_free;

    // if first_free points to 0x8000 now, and it stores 64
    // which is the location to the next free block behind first_free
    // then first_free will point to 0x8040 afterward 
    current_page->first_free = current_page->start_address + *(int *)(current_page->first_free);
    current_page->object_count++;

    // the page is full now
    if (current_page->object_count == allocator->max_object_count)
    {
        list_insert_prev(&current_page->list, &allocator->full);
        allocator->current_page = NULL;//
    }

    int index = (object - current_page->start_address) / allocator->object_size;

    uart_puts("current_page->page_number:");
    uart_hex(current_page->page_number);
    uart_puts("\t");
    uart_puts("allocator->object_size:");
    uart_hex(allocator->object_size);
    uart_puts("\t");
    uart_puts("index:");
    uart_hex(index);
    uart_puts("\r\n");
    
    uart_puts("[obj_malloc] done\r\n");

    return object;
}

void obj_free(void *object)
{
    int page_number = (long)(object - MEMORY_START) >> PAGE_SHIFT;
    struct page *page = &page_list[page_number];
    struct dynamic_allocator *allocator = page->allocator;
    // for example, if we have 4096 * 14 + 32 * 5 as our address, then we get 5 with the following operation
    int index = (((long)(object - MEMORY_START) & ((1 << PAGE_SHIFT) - 1)) / allocator->object_size);

    uart_puts("page->page_number:");
    uart_hex(page->page_number);
    uart_puts("\t");
    uart_puts("allocator->object_size:");
    uart_hex(allocator->object_size);
    uart_puts("\t");
    uart_puts("index:");
    uart_hex(index);
    uart_puts("\r\n");

    // we are freeing the []
    if (object > page->first_free) // object to free is behind the first free 
    {
        // XXOX[]O...
        // the object is between the first hole and the second hole
        if (object < (page->start_address + *(int *)page->first_free))
        {
            uart_puts("[obj_free] status 1\n\n");

            *(int *)object = *(int *)page->first_free;
            *(int *)page->first_free = object - page->start_address;
        }
        // XXOOO[]...
        // the object is behind the second hole
        // so we need to iterate over the list to find the last hole in front of the object
        else
        {
            uart_puts("[obj_free] status 2\n\n");

            void *traversal = page->first_free;
            while ((page->start_address + (*(int *)traversal)) < object)
            {
                traversal = page->start_address + (*(int *)traversal);
            }

            *(int *)object = *(int *)traversal;
            *(int *)traversal = object - page->start_address;
        }
    }
    // XX[]XXO...
    // the object is in front of the first hole
    else
    {
        uart_puts("[obj_free] status 3\n\n");
        
        *(int *)object = page->first_free - page->start_address;
        page->first_free = object;
    }
    
    page->object_count--;

    list_remove(&page->list, &page->list); //remove from full

    // full/partial to partial
    if (page->object_count > 0)
    {
        list_insert_prev(&page->list, &allocator->partial);
    }
    else // empty
    {
        if (page == allocator->current_page)
        {
            allocator->current_page = NULL;
        }
        page_free(page);
    }

    uart_puts("[obj_free] done\r\n");
}

void *memory_allocation(int size)
{
    void *address;

    if (size <= (PAGE_SIZE / 2))
    {
        for (int i = MIN_OBJECT_ORDER; i < MAX_OBJECT_ORDER + 1; i++) 
        {
            if (size <= (1 << i))
            {
                uart_puts("======use dynamic alloc======\r\n");
                address = obj_malloc(i - MIN_OBJECT_ORDER);
                uart_puts("--------------------\r\n");
                return address;
            }
        }
    }
    else
    {
        for (int i = 0; i < MAX_BUDDY_ORDER; i++)
        {
            if (size <= (1 << (i + PAGE_SHIFT)))
            {
                uart_puts("======use buddy system======\r\n");
                address = page_alloc(i)->start_address;
                uart_puts("--------------------\r\n");
                return address;
            }
        }
    }

    uart_puts("[memory_allocation] the requested size is too large!\n");
    return 0;
}

void memory_free(void *address)
{
    int page_number = (long)(address - MEMORY_START) >> PAGE_SHIFT;
    struct page *page = &page_list[page_number];

    if (page->allocator)
        obj_free(address);
    else
        page_free(page);

    uart_puts("--------------------\n\n");
}

int find_buddy(int page_number, int order)
{
    return page_number ^ (1 << order);
}

void mm()
{

    // memory_init();

    uart_puts("===============before allocation====================================");

    print_buddy_info();

    uart_puts("===============first allocate start====================================");

    void *address_1 = memory_allocation(16);

    print_buddy_info();

    uart_puts("===============first free start====================================");

    memory_free(address_1);

    print_buddy_info();

    uart_puts("===============second allocate start====================================");
    
    void *address_2 = memory_allocation(16384);

    print_buddy_info();

    uart_puts("===============second free start====================================");

    memory_free(address_2);

    print_buddy_info();
}

void print_buddy_info(){
    uart_puts("*********************************************\n");
    uart_puts("buddy system\n");
    uart_puts("order\tfree_page\n");
    for (int i = 0; i <= MAX_BUDDY_ORDER; i++) 
    {
        list_head_t *l = &(free_buddy_list[i]);
        list_head_t *head = l;
        int count = 0;
	    uart_put_int(i);
	    uart_puts("\t");
        while (l->next != head) 
        {
        l = l->next;
        count++;
        }
        uart_put_int(count);
	    uart_puts("\n");
    }
    uart_puts("*********************************************\n");
}

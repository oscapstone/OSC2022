
#include "mm.h"

void mm_init()
{
    cur_mm_addr = NULL;
}

void get_one_page()
{
    cur_mm_addr = buddy_alloc(0);
    struct page_header *head = (struct page_header *) cur_mm_addr;
    head->byte_used = sizeof(struct page_header);
    head->remain_size = PAGE_SIZE - sizeof(struct page_header);
    cur_mm_addr += sizeof(struct page_header);
}

void* mm_alloc(size_t size)
{
    size_t align_size = ALIGN(size + sizeof(size_t), sizeof(size_t));
    struct page_header *head;
    void *alloc_addr;
    
    uart_sdec("Allocating request size = ", size, " bytes. ");
    uart_sdec("Aligned size = ", align_size, " bytes.\n");

    if (align_size > PAGE_SIZE - sizeof(struct page_header)) {
        // request size is larger than one page size - page header size
        alloc_addr = buddy_alloc(log2_ceiling(align_size) - log2_ceiling(PAGE_SIZE));
        *((size_t *) alloc_addr) = align_size;
        alloc_addr += sizeof(size_t);

        uart_shex("Aligned size is larger than one page size - page header size. 0x", alloc_addr, " is allocated for you. \n\n");
    }
    else {
        if (cur_mm_addr == NULL) {
            get_one_page();
        }
        head = get_page_head(cur_mm_addr);

        if (align_size > head->remain_size) {
            get_one_page();
            head = get_page_head(cur_mm_addr);
            uart_puts("Aligned size is larger than remain space.\n");
        }
        alloc_addr = cur_mm_addr + sizeof(size_t);
        *((size_t *) cur_mm_addr) = align_size;
        cur_mm_addr += align_size;
        head->byte_used += align_size;
        head->remain_size -= align_size;

        uart_shex("0x", alloc_addr, " is allocated for you. \n\n");
    }

    return alloc_addr;
}

void mm_free(void* addr)
{
    size_t data_size = *((size_t *) (addr - sizeof(size_t)));
    struct page_header *head = get_page_head(addr);

    uart_shex("Freeing address 0x", addr, "\n");

    if (data_size > PAGE_SIZE - sizeof(struct page_header)) {
        uart_puts("Start to free buddy due to freed size larger than page size.\n");
        buddy_free(addr);
    }
    else {
        //uart_shex("head = 0x", head, "\n");
        // uart_shex("cur_mm_addr = 0x", cur_mm_addr, "\n");
        // uart_sdec("head->byte_used = ", head->byte_used, "\n");
        head->byte_used -= data_size;
        // uart_sdec("head->byte_used = ", head->byte_used, "\n");
        // uart_sdec("data_size = ", data_size, "\n");
        if (head->byte_used == sizeof(struct page_header)) {
            if (head == get_page_head(cur_mm_addr)) {
                cur_mm_addr = NULL;
            }
            uart_puts("Start to free buddy due to the page is no longer in used.\n");
            buddy_free(head);
        }
    }
    uart_puts("\n");
}

struct page_header *get_page_head(void* addr)
{
    return (struct page_header *) ((uint32_t) addr & (0xFFFFFFFF - PAGE_SIZE + 0x1));
}
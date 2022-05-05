#include "buddy.h"

// from linker.ld
extern char _heap_start;
extern char _text_start;

unsigned long long int buddy_base;
unsigned long long int buddy_end;
unsigned int frame_ents_size;       // number of frames

frame_ent *frame_ents;              // all page frames
list_head_t freelists[MAX_ORDER];

// frame index to frame address
void *idx_to_addr(int idx) {
    return (void *)(buddy_base + idx * PAGE_SIZE);
}

// frame address to frame index
int addr_to_idx(void *ptr) {
    return ((unsigned long long int)ptr - buddy_base) / PAGE_SIZE;
}

// return x (num <= 2**x)
int num_to_exp(int num) {
    int exp = 0;
    for (int i = 0; i < MAX_ORDER + 1; i++) {
        exp = i;
        if (num <= pow(2, exp))
            break;
    }
    return exp;
}

// buddy system first stage init
void buddy_alloc_init(void *start, void *end) {
    buddy_base = (unsigned long long int)start;
    buddy_end = (unsigned long long int)end;

    frame_ents_size = (buddy_end - buddy_base) / PAGE_SIZE;
    frame_ents = simple_alloc(sizeof(frame_ent) * frame_ents_size);

    for (int i = 0; i < frame_ents_size; i++) {
        frame_ents[i].exp = 0;
        frame_ents[i].allocated = 0;
    }
}

// buddy system second stage init
void page_alloc_init() {
    // Merge buddy system (low order -> high order)
    for (int exp = 0; exp < MAX_ORDER - 1; exp++) {
        // two index that should be merge
        int idx_1 = 0, idx_2 = 0;
        while (1) {
            // set second index
            idx_2 = idx_1 ^ (1 << exp);
            if (idx_2 >= frame_ents_size)
                break;
            
            // if these 2 frames are not allocated, and have same exponential -> merge
            if (
                !frame_ents[idx_1].allocated && !frame_ents[idx_2].allocated &&
                frame_ents[idx_1].exp == exp && frame_ents[idx_2].exp == exp
            ) {
                frame_ents[idx_1].exp = exp + 1;
            }

            // set first index
            idx_1 += (1 << (exp + 1));
            if (idx_1 >= frame_ents_size)
                break;
        }
    }

    // init freelist
    for (int i = 0; i < MAX_ORDER; i++)
        INIT_LIST_HEAD(&freelists[i]);

    // update freelists
    int order = 0;
    for (int idx = 0; idx < frame_ents_size; idx += (1 << order)) {
        order = frame_ents[idx].exp;
        // if this frame is not allocated -> add to freelist
        if (!frame_ents[idx].allocated) {
            frame_hdr *hdr;
            hdr = idx_to_addr(idx);
            list_add_tail(&hdr->list, &freelists[order]);
        }
    }
}

void *alloc_pages(int num) {
    // uart_printf_async("------------ In function alloc_pages(%d) ------------\r\n", num);
    int idx, exp, alloc_exp;
    frame_hdr *hdr;

    if (num <= 0)
        return 0;
    
    exp = num_to_exp(num);

    if (exp >= MAX_ORDER)
        return 0;
    
    // find freed page which size is larger than require
    for (alloc_exp = exp; alloc_exp < MAX_ORDER; alloc_exp++)
        if (!list_empty(&freelists[alloc_exp]))
            break;

    // there is no space
    if (alloc_exp == MAX_ORDER)
        return 0;

    // start allocate
    hdr = (frame_hdr *)freelists[alloc_exp].next;
    idx = addr_to_idx(hdr);
    list_del_entry(&hdr->list);

    // release redundant memory block
    while (alloc_exp != exp) {
        int buddy_idx;
        frame_hdr *buddy_hdr;

        alloc_exp -= 1;
        buddy_idx = idx ^ (1 << alloc_exp);

        frame_ents[buddy_idx].exp = alloc_exp;
        frame_ents[buddy_idx].allocated = 0;

        buddy_hdr = idx_to_addr(buddy_idx);
        list_add(&buddy_hdr->list, &freelists[alloc_exp]);
        // uart_printf_async("[-] Release redundant memory (idx : %d -> %d), this block has exp : %d\r\n", idx, buddy_idx, alloc_exp);
    }

    frame_ents[idx].exp = exp;
    frame_ents[idx].allocated = 1;
    // uart_printf_async("[+] Successfully allocate (idx : %d, exp : %d)\r\n", idx, exp);
    return (void *)hdr;
}

static inline void _free_page(frame_hdr *page) {
    int idx, buddy_idx, exp;
    idx = addr_to_idx(page);
    exp = frame_ents[idx].exp;
    frame_ents[idx].allocated = 0;
    list_add(&page->list, &freelists[exp]);

    buddy_idx = idx ^ (1 << exp);
    // merge
    while (exp < MAX_ORDER - 1 && !frame_ents[buddy_idx].allocated && frame_ents[buddy_idx].exp == exp) {
        // uart_printf_async("[*] Coalesce blocks (idx : %d & %d), and their new exp is %d\r\n", idx, buddy_idx, exp + 1);
        frame_hdr *hdr;
        exp += 1;
        hdr = idx_to_addr(idx);
        list_del_entry(&hdr->list);
        hdr = idx_to_addr(buddy_idx);
        list_del_entry(&hdr->list);

        idx = idx & buddy_idx;
        hdr = idx_to_addr(idx);
        frame_ents[idx].exp = exp;
        list_add(&hdr->list, &freelists[exp]);
        buddy_idx = idx ^ (1 << exp);
    }
}

void free_page(void *page) {
    // uart_printf_async("++++++++++++ In function free_page(idx = %d) ++++++++++++\r\n", addr_to_idx(page));
    _free_page((frame_hdr *)page);
}

void memory_reserve(void *start, void *end) {
    uart_printf("Reserve => 0x%x ~ 0x%x\r\n", (unsigned long long int)start, (unsigned long long int)end);
    start = (void *)((unsigned long long int)start & ~(PAGE_SIZE - 1));
    end = (void *)ALIGN((unsigned long long int)end, PAGE_SIZE);
    // allocated all pages from start to end
    while (start < end) {
        int idx = addr_to_idx(start);
        frame_ents[idx].allocated = 1;
        start = (void *)((unsigned long long int)start + PAGE_SIZE);
        // uart_printf_async("Reserve page idx : %d, address : 0x%x\r\n", idx, idx_to_addr(idx));
    }
}

void mm_init() {
    // buddy system first stage init
    buddy_alloc_init((void *)0, (void *)0x3c000000);
    // small chunk first stage init
    sc_alloc_init();
    // Spin tables for multicore boot
    memory_reserve((void *)0, (void *)0x1000);
    // Kernel image in the physical memory
    memory_reserve(&_text_start, &_heap_start);
    // Initramfs
    memory_reserve(INITRD_ADDR, INITRD_END);
    // Device tree (qemu = 83580, rpi3 = 32937)
    memory_reserve(DTB_ADDRESS, DTB_ADDRESS + 83580);
    // for simple_alloc
    memory_reserve((void *)0x2c000000, (void *)0x2e000000);
    // for kernel stack
    memory_reserve((void *)0x2e000000, (void *)0x3c000000);
    // buddy system second stage init
    page_alloc_init();
    // small chunk second stage init
    sc_init();
}

void page_allocator_test()
{
    // allocate all fragments
    char *ptr1 = alloc_pages(2);    // idx = 2
    char *ptr2 = alloc_pages(1);    // idx = 1
    char *ptr3 = alloc_pages(4);    // idx = 4
    char *ptr4 = alloc_pages(2);    // idx = 138
    char *ptr5 = alloc_pages(2);    // idx = 32786
    char *ptr6 = alloc_pages(1);    // idx = 32785

    // uart_printf_async("------------------------------------------------------------\r\n");

    // test alloc_pages -> release redundant memory
    free_page(ptr3);                // idx = 4      (idx 5, 6, 7 are also freed)
    char *ptr7 = alloc_pages(1);    // idx = 4      (split ptr3)
    char *ptr8 = alloc_pages(2);    // idx = 6
    char *ptr9 = alloc_pages(1);    // idx = 5

    // uart_printf_async("------------------------------------------------------------\r\n");

    // test free_page -> coalesce blocks
    free_page(ptr7);
    free_page(ptr8);
    free_page(ptr9);

    // uart_printf_async("------------------------------------------------------------\r\n");

    // free all pointer
    free_page(ptr1);
    free_page(ptr2);
    free_page(ptr4);
    free_page(ptr5);
    free_page(ptr6);
}

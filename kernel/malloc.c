#include "malloc.h"

unsigned int sc_sizes[] = {
    0x10, 0x20, 0x30, 0x40, 0x60,
    0x80, 0xc0, 0x100, 0x400, 0x1000
};

typedef struct {
    unsigned int size_idx;
    unsigned int splitted;
} sc_frame_ent;

typedef struct {
    list_head_t list;
} sc_hdr;

sc_frame_ent *sc_frame_ents;
list_head_t sc_freelists[ARRAY_SIZE(sc_sizes)];

unsigned int find_size_idx(int size) {
    for (unsigned int i = 0; i < ARRAY_SIZE(sc_sizes); i++) {
        if (size <= sc_sizes[i])
            return i;
    }
    return -1;
}

// small chunk first stage init
void sc_alloc_init() {
    sc_frame_ents = simple_alloc(sizeof(sc_frame_ent) * frame_ents_size);
    for (int i = 0; i < frame_ents_size; i++)
        sc_frame_ents[i].splitted = 0;
}

// small chunk second stage init
void sc_init() {
    for (int i = 0; i < ARRAY_SIZE(sc_sizes); i++)
        INIT_LIST_HEAD(&sc_freelists[i]);
}

void *sc_alloc(int size) {
    if (size > 0x1000) {
        uart_printf("Error : sc_alloc(size), size > 0x1000\r\n");
        return 0;
    }

    sc_hdr *hdr;
    unsigned int size_idx = find_size_idx(size);

    // if there is no free chunk (same size), allocate a page and split it into small chunks
    if (list_empty(&sc_freelists[size_idx])) {
        void *page = alloc_pages(1);
        int frame_idx = addr_to_idx(page);

        sc_frame_ents[frame_idx].size_idx = size_idx;
        sc_frame_ents[frame_idx].splitted = 1;

        // cut the entire page into chunks
        for (int i = 0; i + sc_sizes[size_idx] <= PAGE_SIZE; i += sc_sizes[size_idx]) {
            hdr = (sc_hdr *)((char *)page + i);
            list_add_tail(&hdr->list, &sc_freelists[size_idx]);
        }
    }

    // allocate this chunk and remove from free list
    hdr = (sc_hdr *)sc_freelists[size_idx].next;
    list_del_entry(&hdr->list);

    return hdr;
}

int sc_free(void *sc) {
    sc_hdr *hdr;
    int frame_idx = addr_to_idx(sc);
    int size_idx;

    // this chunk is not managed by Small Chunk allocator
    if (!sc_frame_ents[frame_idx].splitted) {
        return -1;
    }
    
    // add this chunk to free list
    size_idx = sc_frame_ents[frame_idx].size_idx;
    hdr = (sc_hdr *)sc;
    list_add(&hdr->list, &sc_freelists[size_idx]);

    return 0;
}

// test for small chunk allocator
void sc_test() {
    char *ptr1 = sc_alloc(0x18); // A; Allocate a page and create 0x20 chunks
    char *ptr2 = sc_alloc(0x19); // B
    char *ptr3 = sc_alloc(0x1a); // C

    sc_free(ptr1); // 0x20 freelist: A
    sc_free(ptr3); // 0x20 freelist: C -> A

    char *ptr4 = sc_alloc(0x17); // C; 0x20 freelist: A
    char *ptr5 = sc_alloc(0x17); // A; 0x20 freelist: <NULL>

    sc_free(ptr2);
    sc_free(ptr4);
    sc_free(ptr5);

    char *ptr6 = sc_alloc(0x369); // Allocate a page and create 0x400 chunks
    sc_free(ptr6);
}

void *kmalloc(int size) {
    void *ret;

    lock();

    if (size <= PAGE_SIZE) {
        ret = sc_alloc(size);
    }
    else {
        int page_cnt = ALIGN(size, PAGE_SIZE) / PAGE_SIZE;
        ret = alloc_pages(page_cnt);
    }

    unlock();

    return ret;
}

void kfree(void *ptr) {
    lock();

    if (!sc_free(ptr)) {
        unlock();
        return;
    }

    free_page(ptr);
    unlock();
    return;
}

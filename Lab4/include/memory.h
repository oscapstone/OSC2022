#ifndef	_MEMORY_H
#define	_MEMORY_H

#include <stdint.h>
#include <stddef.h>

// 0x8000_0 -> 0x4000_00   kernel
// 0x1000_0000 -> 0x2000_0000

#define INIT_VAL 4
#define FRAME_ARRAY_SIZE (1 << INIT_VAL)
#define MAX_32K_NUM (FRAME_ARRAY_SIZE / 8)
#define MEMORY_BASE_ADDR 0x10000000
#define GET_PAGE_ADDR(index) (MEMORY_BASE_ADDR + (index << 12))
#define GET_PAGE_INDEX(addr) ((addr - MEMORY_BASE_ADDR) >> 12)

typedef struct frame_free_node {
    uint64_t index;
    struct frame_free_node *next;
    struct frame_free_node *prev;   // double linked list to enable O(1) removal
} frame_free_node;

void memory_init();
uint64_t page_malloc();
uint64_t request_page(int size);
void page_free(uint64_t addr, int size);
void merge_page(uint64_t index, int size);
void pop_front(frame_free_node **list);
void remove_from_list(frame_free_node **list, uint64_t index);
void add_to_list(frame_free_node **list, uint64_t index);
uint64_t getIndex(uint64_t addr, int size);
frame_free_node *get_free_node();
void return_free_node(frame_free_node *node);

void print_frame_array();
void print_frame_free_lists();

#endif

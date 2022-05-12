#ifndef MM_H
#define MM_H
#include "list.h"

#define PAGE_SIZE 4096
#define PAGE_SIZE_CTZ 12 //__builtin_ctz(4096)
#define MEMORY_SIZE (unsigned long)0x40000000
#define MAX_BUDDY_ORDER 16
#define INIT_PAGE 0b00010000 //use 3 bits as state of page and 5 bit as order of page
#define NULL ((void *)0)
#define STARTUP_MAX 16  	// max reserve slot

#define get_order(ptr) (__builtin_ctzl((unsigned long)ptr) - PAGE_SIZE_CTZ)
#define set_buddy_ord(bd, ord) (bd = ord | (bd & 0xe0))
#define set_buddy_flag(bd, flag) (bd = (flag << 5) | (bd & 0x1f))
#define get_buddy_flag(bd) ((bd & 0xe0) >> 5)
#define get_buddy_ord(bd) (bd & 0x1f)
#define pagenum_to_ptr(pgn) ((void *)(((unsigned long)pgn) << PAGE_SIZE_CTZ))
#define pad(x, y) ((((x) + (y)-1) / (y)) * (y))
#define ptr_to_pagenum(ptr) (((unsigned long)(ptr)) >> PAGE_SIZE_CTZ)
#define buddy_pagenum(pg, ord) ((pg) ^ (1 << ord))

typedef struct startup_allocator {
  unsigned long addr[STARTUP_MAX];
  unsigned long size[STARTUP_MAX];
} startup_allocator;

typedef struct buddy_system {
	_list free_list[MAX_BUDDY_ORDER];
} buddy_system;

enum {
	BUDDY_FREE,  			//0
	BUDDY_USE,				//1
	RESERVE_USE
};
buddy_system buddy;
char* buddy_state;
startup_allocator startup;

void init_buddy_system();
void init_memory_system();
void place_buddy(unsigned long st, unsigned long ed, int flag);
void _place_buddy(unsigned long ptr, int ord, int flag);
void print_buddy_info();
void print_buddy_stat();
void *page_allocate(unsigned int size);
void page_free(void *ptr);
void test_buddy();
int reserve_mem(unsigned long addr, unsigned long size);
unsigned long check_reserve_collision(unsigned long a1, unsigned int s1, unsigned long a2, unsigned int s2);
void init_startup();
#endif
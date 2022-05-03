#ifndef __ALLOCATOR__H__
#define __ALLOCATOR__H__
#include "uart.h"
#include "buddy.h"

#define OBJ_ALLOCATOR_PAGE_SIZE 		16				// pool allow allocate max page count
#define ALLOCATOR_POOL_SIZE				32

typedef struct 
{
	unsigned int max_pool[OBJ_ALLOCATOR_PAGE_SIZE];		// record max obj slot count (page size / obj size)
	unsigned int cur_pool[OBJ_ALLOCATOR_PAGE_SIZE];		// record current obj use slot count
	unsigned int page[OBJ_ALLOCATOR_PAGE_SIZE];			// record page index
	unsigned int page_count;							// record page count
	unsigned int obj_size;								// obj size
	unsigned int max_pool_init;							// initial max_pool
} object_allocator;

// has 4 different obj size
typedef struct 
{
	object_allocator *obj_allocator_16;
	object_allocator *obj_allocator_32;
	object_allocator *obj_allocator_64;
	object_allocator *obj_allocator_128;
	object_allocator *obj_allocator_256;
	object_allocator *obj_allocator_512;
	object_allocator *obj_allocator_1024;
	object_allocator *obj_allocator_2048;
	object_allocator *obj_allocator_4096;
} dynamic_allocator;

object_allocator* object_allocator_init(int size);
unsigned long long obj_alloc(object_allocator *self);
int obj_allocator_free(object_allocator *self, unsigned long long addr);

dynamic_allocator* dynamic_allocator_init();
unsigned long long dynamic_alloc(dynamic_allocator *self, int req_size);
void dynamic_free(dynamic_allocator *self, unsigned long long addr);
void dynamic_test();

void memory_init();
#endif
#include "stdlib.h"
#define _smalloc_size 0xffffff
extern unsigned long _head_start_brk;
void* heap_top = &_head_start_brk;
void* simple_malloc(size_t size) {
  void* last_heap_top = heap_top;
  heap_top += size;
  return last_heap_top;
}

void* get_smalloc_end(){
  void* smalloc_start = &_head_start_brk;
  return (void*)((unsigned long long)smalloc_start+_smalloc_size);
}
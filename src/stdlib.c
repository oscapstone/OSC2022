#include "stdlib.h"
extern unsigned long _head_start_brk;
void* heap_top = &_head_start_brk;
void* simple_malloc(size_t size) {
  void* last_heap_top = heap_top;
  heap_top += size;
  return last_heap_top;
}
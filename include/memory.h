#ifndef MEMORY
#define MEMORY

#define NULL 0x00000000;

extern unsigned long _heap_start; // linker script symbol
extern unsigned long _heap_end;  // linker script symbol
static char* heap = (char*)&_heap_start;
static char* heap_end = (char*)&_heap_end;

void* simple_malloc(unsigned int size);

#endif
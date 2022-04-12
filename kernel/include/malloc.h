#ifndef MALLOC_H_
#define MALLOC_H_
#include <list.h>
#include <stddef.h>
#define MALLOC_BASE   ((volatile unsigned long*)(0x5000000))
#define MAX_CHUNK_SIZE 11

typedef struct {
    struct list_head list;
}FreeChunkList;

typedef struct {
    struct list_head list; // size 0x10
}Chunk;

unsigned int find_level(unsigned int);
void *simple_malloc(unsigned long);
void freechunk_list_init();
void *chunk_alloc(unsigned int);
void chunk_free(void *);


void print_freechunk_list();

#endif
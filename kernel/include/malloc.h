#ifndef MALLOC_H_
#define MALLOC_H_
#include <list.h>
#include <stddef.h>
#define MALLOC_BASE   ((volatile unsigned long*)(0x5000000))
#define MAX_CHUNK_SIZE 10

typedef struct {
    struct list_head list;
}FreeChuckList;

typedef struct {
    struct list_head list;
    unsigned int size;
    unsigned int idx;
    unsigned int free;
}Chunk;

int find_level(unsigned int);
void *simple_malloc(unsigned long);
void freechunk_list_init();


#endif
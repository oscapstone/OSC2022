#ifndef	_MM_H
#define	_MM_H

#define MEM_REGION_BEGIN    0x10000000
#define MEM_REGION_END      0x20000000
#define PAGE_SIZE           4096
#define MAX_ORDER           6 // largest: PAGE_SIZE*2^(MAX_ORDER)

#define ALLOCABLE           0
#define ALLOCATED           -1
#define C_NALLOCABLE        -2

#define NULL                (void*)0

void *malloc(unsigned int size);
void free(void *address);
void init_mm();

struct frame {
    unsigned int index;
    int val;
    int state;
    struct frame *prev, *next;    
};

#endif  /*_MM_H */
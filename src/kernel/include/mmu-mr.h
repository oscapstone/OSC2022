#ifndef _DEF_MMU_MR
#define _DEF_MMU_MR
#include <stdint.h>

typedef struct MemoryRegion_{
    uint64_t VA_base;
    uint64_t pages;
    int flags;
    int prot;
    char *filepath;
    struct MemoryRegion_* next;
} MemoryRegion;

#endif
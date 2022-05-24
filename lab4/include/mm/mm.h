#ifndef _MM_H_
#define _MM_H_

#include "types.h"
#include "lib/fdt_parse.h"
#include "lib/print.h"
#include "lib/simple_malloc.h"
#include "lib/list.h"
#include "lib/string.h"
#include "mm/page_alloc.h"
#include "mm/slab.h"


struct mem_block{
    uint64_t start;
    uint64_t end;
    struct list_head list;
};

struct mem_node{
    uint64_t start;
    uint64_t end;
    char *name;
    struct list_head list;
};

extern struct list_head mem_rsvmap;
extern struct list_head mem_unusedmap;
extern struct mem_node memory_node;
extern struct page *mem_map;

extern void mm_init(void *);


#endif

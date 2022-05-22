#ifndef _MM_H_
#define _MM_H_

#include "types.h"
#include "lib/fdt_parse.h"
#include "lib/print.h"
#include "lib/simple_malloc.h"
#include "lib/list.h"
#include "lib/string.h"
#include "mm/page_alloc.h"

#define PAGE_SHIFT 12
#define PAGE_SIZE (1ul << PAGE_SHIFT)

struct mem_block{
    uint64_t start;
    uint64_t end;
    struct list_head list;
};
extern void mm_init(void *);

#endif

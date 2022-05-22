#ifndef _MM_H_
#define _MM_H_
#include "lib/fdt_parse.h"
#include "lib/print.h"
#include "lib/simple_malloc.h"
#include "lib/list.h"
#include "lib/string.h"

struct mem_block{
    uint64_t start;
    uint64_t end;
    struct list_head list;
};
extern void mm_init(void *);

#endif

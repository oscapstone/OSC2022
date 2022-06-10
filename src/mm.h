
#ifndef __MM_H__
#define __MM_H__

#include "stddef.h"
#include "buddy.h"
#include "math.h"
#include "stdlib.h"

// All address should be aligned to sizeof(size_t) bytes.
// This implement may suffer from serious internal fragmentation problem.

void *cur_mm_addr;

// The page header should be placed in the beginning of a page.
struct page_header {
    size_t byte_used; // Indicate the number of bytes used in one page.
    size_t remain_size;
};

//   ------------------------------------------------------------------------------------------------
//  |   page_header   |   freed   |      allocated      |                remain_size                 |
//   ------------------------------------------------------------------------------------------------


void get_one_page();
struct page_header *get_page_head(void* addr);

#endif
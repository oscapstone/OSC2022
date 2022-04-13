#include <stdint.h>
#include <string.h>
#include "mem.h"
#include "mini_uart.h"

#define NULL 0

struct page {
  uint64_t size;
  struct page *next_page;
  uint64_t bitmap[4];
};


void* register_slot(struct page* p) {
  uint64_t size = p->size;

  int flag = 0;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 64; j++) {
      int idx = i * 64 + j;
      if (idx * size + sizeof(struct page) >= FRAME_SIZE) {
        flag = 1;
        break;
      }

      if (!(p->bitmap[i] & (1<<j))) {
        p->bitmap[i] |= (1<<j);
        void *alloc_ptr = ((void*)p) + sizeof(struct page) + size * idx;
        memset(alloc_ptr, 0, size);
        return alloc_ptr;
      }
      
    }
    if (flag) break;
  }

  p->next_page = (struct page*)((void*)MEMORY_BASE + FRAME_SIZE * allocate_frame(0));
  memset(p->next_page, 0, sizeof(struct page));
  p->size = size;
  return register_slot(p->next_page);
}

static struct page *pages[6];

void* kmalloc(uint32_t size) {
  // 16 32 64 128 256 512
  for (uint32_t k = 0; k < 6; k++) {
    uint32_t sz = 1 << (k + 4);
    if (sz > size) {
      if (pages[k] == NULL) {
        pages[k] = (struct page*)((void*)MEMORY_BASE + FRAME_SIZE * allocate_frame(0));
        memset(pages[k], 0, sizeof(struct page));
        pages[k]->size = 1 << (k+4);
      }
      return register_slot(pages[k]);
      break;
    }
  }
  return NULL;
}

void kfree(void *ptr) {
  struct page *pg = (struct page*)((uint64_t)ptr & (~(FRAME_SIZE-1)));
  uint64_t size = pg->size;
  int idx = ((uint64_t)(ptr - (void*)pg - sizeof(struct page))) / size;
  if (pg->bitmap[idx/64] & (1<<(idx&63))) {
    pg->bitmap[idx/64] ^= (1<<(idx&63));
  }

  for (int i = 0; i < 4; i++) {
    if (pg->bitmap[i] != 0) return;
  }
  for (int i = 0; i < 6; i++) {
    if (pages[i] == pg) {
      pages[i] = pg->next_page;
      deallocate_frame((uint64_t)pg / FRAME_SIZE);
    }
  }
}

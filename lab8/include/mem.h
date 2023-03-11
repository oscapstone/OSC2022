#ifndef MEM_H
#define MEM_H

#define MEMORY_BASE  0x00000000l
// #define MEMORY_LIMIT 0x2C000000
#define FRAME_SIZE 0x1000

#define MAX_EXP 7

struct pageBlock {
  void* vBegin;
  int exp;
};

void init_frame_allocator();
int allocate_frame(int reqexp);
int deallocate_frame(int idx);

unsigned long long align_addr(unsigned long long addr);
int physicalToIndex(unsigned long addr);
int kVirtualToIndex(unsigned long addr);
void memory_reserve(int start_idx, int count);

void *getFreePage();
void *getContFreePage(int cnt, int *alloc_exp);

void *kmalloc(unsigned int size);
void kfree(void *ptr);

void *smalloc(unsigned int size);


#endif

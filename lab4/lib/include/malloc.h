
extern unsigned long _kernel_start;
extern unsigned long _kernel_end;
extern unsigned char _heap_start;

typedef unsigned long size_t;

typedef struct frame_info {
  unsigned int status;
  int index;
} frame_info;

typedef struct page_item {
  int index;
  struct page_item *previous;
  struct page_item *next;
} page_item;

typedef struct pool_header {
  unsigned int size;
  unsigned int total;
  unsigned int used;
  struct pool_header *next;
} pool_header;

void* simple_malloc(size_t size);
void page_init();
int page_allocate(size_t size);
void page_free(int index);
void show_frame();
void show_page_list();

void *malloc(size_t size);
void free(void *addr);

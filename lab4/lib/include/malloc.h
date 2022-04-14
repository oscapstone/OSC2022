typedef unsigned long size_t;

typedef struct malloc_header {
  unsigned int previous;
  unsigned int chunk;
} malloc_header;

void* simple_malloc(size_t size);


typedef struct frame_list {
  unsigned int index;
  unsigned int queue_index;
  struct frame_list *next;
} frame_list;

void page_init();

int page_allocate(size_t size);
void page_free(int index);

void *malloc(size_t size);
void free(char *addr);

void memory_reserve(unsigned long start, unsigned long end);

void print_frame_state();
void print_free_frame_list();

typedef struct malloc_mem{
  unsigned int allocted;
  unsigned int size;
  unsigned int previous;
} malloc_mem;


typedef struct allocated_page{
  unsigned long index;
  unsigned long total;
  unsigned long free;
  struct allocated_page *next;
} allocated_page;

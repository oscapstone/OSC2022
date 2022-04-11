typedef unsigned long size_t;

typedef struct malloc_header {
  unsigned int previous;
  unsigned int chunk;
} malloc_header;

void* simple_malloc(size_t size);



#define PAGE_MAX_ENTRY 50
#define MAX_CONTINUE_PAGE 32

#define PAGE_NOT_ALLOCATE -1  // for the frame array which is not allocated
#define PAGE_ALLOCATED -2  // for the frame array is allocated

#define MAX_FREE_ITEM (PAGE_MAX_ENTRY)  // manage the free item fifo array

typedef struct frame_list {
  unsigned int index;
  unsigned int queue_index;
  struct frame_list *next;
} frame_list;

void page_init();

void *malloc(size_t size);
void free(char *addr);

void print_frame_state();
void print_free_frame_list();

typedef struct malloc_mem{
  int allocted;
  unsigned long size;
  unsigned long previous;
} malloc_mem;


typedef struct allocated_page{
  unsigned long index;
  unsigned long total;
  unsigned long free;
  struct allocated_page *next;
} allocated_page;

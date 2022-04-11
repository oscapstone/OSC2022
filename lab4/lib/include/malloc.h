typedef unsigned long size_t;

typedef struct malloc_header {
  unsigned int previous;
  unsigned int chunk;
} malloc_header;

void* simple_malloc(size_t size);



#define PAGE_MAX_ENTRY 56
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

int page_allocate(size_t size);
void page_free(int index);

void print_frame_state();
void print_free_frame_list();


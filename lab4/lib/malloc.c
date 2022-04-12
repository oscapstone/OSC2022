#include "malloc.h"
#include "printf.h"


extern unsigned char _heap_start;
static char* top = (char*) 0;
static char* simple_mallc_top = (char*) &_heap_start;

// #define PAGE_MAX_ENTRY 100

#define MAX_PAGE_ORDER 15

#define PAGE_NOT_ALLOCATE -1  // for the frame array which is not allocated
#define PAGE_ALLOCATED -2  // for the frame array is allocated
#define PAGE_RESERVED -3


unsigned int PAGE_MAX_ENTRY  = 0x3c000000/0x1000;


void* simple_malloc(size_t size) {
  size = 0x10 + size - size%0x10;  // ALIGN 16
  simple_mallc_top += size;
  return simple_mallc_top;
}


int *frame;
// int frame[PAGE_MAX_ENTRY] = {0}; // frame array for record the status of page

frame_list *free_item;
// frame_list free_item[PAGE_MAX_ENTRY];
int *free_item_queue;
// int free_item_queue[PAGE_MAX_ENTRY];
int queue_front = -1;
int queue_end = -1;

frame_list *free_frame_list[MAX_PAGE_ORDER+1] = {NULL}; // linked-list for free page


allocated_page *page_allocated;
// allocated_page page_allocated[PAGE_MAX_ENTRY];
int *free_page_queue;
// int free_page_queue[PAGE_MAX_ENTRY];
int free_queue_front = -1;
int free_queue_end = -1;

allocated_page *mem_alloc_page = NULL;

static void enqueue_free_item(int value);
static int dequeue_free_item();

static void add_frmae_list(int value, int index);
static int allocate_frame_list(int value);
static void delete_frame_list(int value, int index);

static int page_allocate(size_t size);
static void page_free(int index);

static void page_devide(int value);
static int find_in_free_frame_list(int level, int index);
static void merge(int index);

// function for calculating power of 2 and log of 2
static int get_log_of_two(int value);
static int power_of_two(int value);

static void alloc_page_to_mem(int request);
static void merge_mem(int index, char *addr);

static int dequeue_page_allocate();
static void enqueue_page_allocate();


void page_init(){
  char *sim_alloc_start = 0;
  char *sim_alloc_end = 0;
  // for page alloc
  frame_list *addr = simple_malloc(PAGE_MAX_ENTRY * sizeof(frame_list));
  free_item = addr;
  sim_alloc_start = (char *)addr;
  addr = simple_malloc(PAGE_MAX_ENTRY * sizeof(int));
  frame = (int *)addr;
  addr = simple_malloc(PAGE_MAX_ENTRY * sizeof(int));
  free_item_queue = (int *)addr;
  // for malloc
  addr = simple_malloc(PAGE_MAX_ENTRY * sizeof(allocated_page));
  page_allocated = (allocated_page *)addr;
  addr = simple_malloc(PAGE_MAX_ENTRY * sizeof(int));
  sim_alloc_end = (char *)addr + PAGE_MAX_ENTRY * sizeof(int);
  free_page_queue = (int *)addr;
  
  // init the free queue array for managing the free_item
  for(int i=0; i<PAGE_MAX_ENTRY; i++){
    *(frame+i) = 0;
    enqueue_free_item(i);
    enqueue_page_allocate(i);
  }

  int non_init = PAGE_MAX_ENTRY;
  for(int start=0; start<PAGE_MAX_ENTRY;){
    frame[start] = get_log_of_two(non_init);
    add_frmae_list(frame[start], start);
    start += power_of_two(frame[start]);
    for(int i=PAGE_MAX_ENTRY-non_init+1; i<start; i++)
      frame[i] = PAGE_NOT_ALLOCATE;
    non_init -= power_of_two(get_log_of_two(non_init));
  }
  memory_reserve(0x0000, 0x1000); //spin table
  memory_reserve((unsigned long)sim_alloc_start, (unsigned long)sim_alloc_end);
  print_free_frame_list();
}

void enqueue_free_item(int value){
  if((queue_front+1)%PAGE_MAX_ENTRY == queue_end)
    printf("queue is full\n\r");
  else{
    if(queue_end == -1)
      queue_end = 0;
    queue_front += 1;
    if(queue_front >= PAGE_MAX_ENTRY && queue_end != 0)
      queue_front = 0;
    free_item_queue[queue_front] = value;
  }
}

int dequeue_free_item(){
  int ans = free_item_queue[queue_end];
  if(queue_front == queue_end){
    queue_front = -1;
    queue_end = -1;
    return -1;
  }else{
    queue_end += 1;
    if(queue_end == PAGE_MAX_ENTRY)
      queue_end = 0;
    return ans;
  }
}

void add_frmae_list(int value, int index){
  int queue_index = dequeue_free_item();
  // frame_list *free_frame = &free_item[queue_index];
  frame_list *free_frame = free_item+queue_index;
  free_frame->index = index;
  free_frame->queue_index = queue_index;
  free_frame->next = NULL;
  if(value <= MAX_PAGE_ORDER && value >=0){
    if(!free_frame_list[value]){
      free_frame_list[value] = free_frame;
    }else{
      free_frame->next = free_frame_list[value];
      free_frame_list[value] = free_frame;
    }
  }else
    printf("Out of rnage!!!\n\r");
}

int allocate_frame_list(int value){
  int ans = -1;
  if(value <= MAX_PAGE_ORDER && value >= 0){
    enqueue_free_item(free_frame_list[value]->queue_index);
    for(int i=1; i<power_of_two(value); i++)
      frame[free_frame_list[value]->index+i] = PAGE_ALLOCATED;
    ans = free_frame_list[value]->index;
    free_frame_list[value] = free_frame_list[value]->next;
    return ans;
  }else
    return -1;
}

void page_devide(int value){
  int index;
  if(free_frame_list[value]){
    enqueue_free_item(free_frame_list[value]->queue_index);
    index = free_frame_list[value]->index;
    free_frame_list[value] = free_frame_list[value]->next;
    frame[index]= value-1;
    add_frmae_list(value-1, index);
    frame[index+power_of_two(value-1)]= value-1;
    add_frmae_list(value-1, index+power_of_two(value-1));
  }else{
    if(value < 5){
      page_devide(value+1);
      page_devide(value);
    }else
      printf("No page!!!\n\r");
  }
}

int page_allocate(size_t size){
  int level = get_log_of_two((size-1)/0x1000)+1;
  if(!free_frame_list[level]){
    if(level < MAX_PAGE_ORDER)
      page_devide(level+1);
    else{
      printf("Request too LARGE size!!\n\r");
      return -1;
    }
  }
  return allocate_frame_list(level);
}

void page_free(int index){
  int size = frame[index];
  add_frmae_list(size, index);
  for(int i=1; i<power_of_two(size); i++){
    frame[index+i] = PAGE_NOT_ALLOCATE;
  }
  merge(index);
}

void delete_frame_list(int value, int index){
  frame_list *pre, *cur;
  cur = free_frame_list[value];
  pre = cur;
  while (cur->index != index){
    pre = cur;
    cur = cur->next;
  }
  enqueue_free_item(cur->queue_index);
  if(pre!=cur)
    pre->next = cur->next;
  else
    free_frame_list[value] = free_frame_list[value]->next;
}

int find_in_free_frame_list(int level, int index){
  frame_list *pre = free_frame_list[level];
  while (pre){
    if(pre->index == index)
      return 1;
    pre = pre->next;
  }
  return 0;
}

void merge(int index){
  if(frame[index] >= MAX_PAGE_ORDER)
   return;
  int size = power_of_two(frame[index]);
  int merge_index = index + size;
  if(index%(2*size))
    merge_index = index - size;
  if(find_in_free_frame_list(frame[index], merge_index) && frame[index] == frame[merge_index]){
    delete_frame_list(frame[index], index);
    delete_frame_list(frame[index], merge_index);
    add_frmae_list(frame[index]+1, merge_index);
    frame[merge_index] = frame[index]+1;
    frame[index] = PAGE_NOT_ALLOCATE;
    merge(merge_index);
  }else
    return;
}

int get_log_of_two(int value){
  int ans = 1;
  for(int i=0; ;i++){
    if(ans > value){
      return (--i)>MAX_PAGE_ORDER?MAX_PAGE_ORDER:i;
    }  // set the max continue order
    ans *= 2;
  }
}

int power_of_two(int value){
  int ans = 1;
  for(int i=0; i<value; i++)
    ans *= 2;
  return ans;
}

void print_frame_state(){
  printf("--------------frame state--------------\n\r");
  for(int i=0; i<PAGE_MAX_ENTRY; i++){
    printf("frmae[%2d] = %2d, ", i, frame[i]);
    if(i%4 == 3 || i+1 == PAGE_MAX_ENTRY)
      printf("\n\r");
  }
}

void print_free_frame_list(){
  printf("--------------free frame list--------------\n\r");
  frame_list *pre;
  for(int i=0; i<=MAX_PAGE_ORDER; i++){
    pre = free_frame_list[i];
    printf("list%d: ", i);
    while (pre){
      printf("%d ", pre->index);
      pre = pre->next;
    }
    printf("\n\r");
  }
}


void *malloc(size_t size){
  int page_request = size + 0x1000 - size%0x1000;
  if(!mem_alloc_page){
    alloc_page_to_mem(page_request);
  }

  /* find the free page */

  allocated_page *cur = mem_alloc_page;
  while(cur){
    if(cur->free >= size){
      break;
    }
    if(!cur->next){
      alloc_page_to_mem(page_request);
    }
    cur = cur->next;
  }

  /*  find the size in page */
  
  malloc_mem *page = (malloc_mem *)(cur->index * 0x1000 + top);
  malloc_mem *last_addr = (malloc_mem *)((unsigned long)page + cur->total);
  while(page->size < size || page->allocted == 1){
    page = (malloc_mem *)((unsigned long)page + page->size + sizeof(malloc_mem));
    if(page == last_addr)
      break;
  }
  if(page < last_addr){
    malloc_mem *last_or_not = (malloc_mem *)((unsigned long)page + page->size + sizeof(malloc_mem));
    page->allocted = 1;
    if(last_or_not == last_addr){
      if(cur->free >= size+sizeof(malloc_mem))
        cur->free = cur->free - size - sizeof(malloc_mem);
      else
        cur->free = 0;
      int remain_size = page->size - size - sizeof(malloc_mem);
      page->size = size;
      page = (malloc_mem *)((unsigned long)page + page->size + sizeof(malloc_mem));
      page->size = remain_size;
      page->previous = size;
      page->allocted = 0;
      return (malloc_mem *)((unsigned long)page - page->previous);
    }
    return (malloc_mem *)((unsigned long)page + sizeof(malloc_mem));
  }else{
    cur = mem_alloc_page;
    while(cur){
      if(!cur->next)
        break;
      cur = cur->next;
    }
    page = (malloc_mem *)(cur->index * 0x1000 + top);
    if(cur->free >= size+sizeof(malloc_mem))
      cur->free = cur->free - size - sizeof(malloc_mem);
    else
      cur->free = 0;
    int remain_size = page->size - size - sizeof(malloc_mem);
    page->allocted = 1;
    page->size = size;
    page = (malloc_mem *)((unsigned long)page + page->size + sizeof(malloc_mem));
    page->size = remain_size;
    page->previous = size;
    page->allocted = 0;
    return (malloc_mem *)((unsigned long)page - page->previous);
  }
}

void free(char *addr){
  int index = (addr-top)/0x1000;
  malloc_mem *cur = (malloc_mem *)((unsigned long)addr - sizeof(malloc_mem));
  cur->allocted = 0;
  allocated_page *page_list = mem_alloc_page;
  while(page_list->next){
    if(page_list->index == index)
      break;
    page_list = page_list->next;
  }
  page_list->free += cur->size;
  merge_mem(index, addr);
  if(page_list->free == (page_list->total - sizeof(malloc_mem))){
    page_free(index);
  }
}

void merge_mem(int index, char *addr){
  allocated_page *page_list = mem_alloc_page;
  while(page_list->next){
    if(page_list->index == index)
      break;
    page_list = page_list->next;
  }
  malloc_mem * cur = (malloc_mem *)((unsigned long)addr - sizeof(malloc_mem));
  malloc_mem * next = (malloc_mem *)((unsigned long)addr + cur->size);
  if(next != (malloc_mem *)(page_list->index * 0x1000 + top + page_list->total)){
    if(next->allocted == 0){
      cur->size += next->size + sizeof(malloc_mem);
      page_list->free += sizeof(malloc_mem);
      next = (malloc_mem *)((unsigned long)next + next->size + sizeof(malloc_mem));
      if(next != (malloc_mem *)(page_list->index * 0x1000 + top + page_list->total))
        next->previous = cur->size;
    }
  }

  if(cur != (malloc_mem *)(page_list->index * 0x1000 + top)){
    malloc_mem * pre = (malloc_mem *)((unsigned long)cur - cur->previous - sizeof(malloc_mem));
    if(pre->allocted == 0){
      pre->size += cur->size + sizeof(malloc_mem);
      page_list->free += cur->size + sizeof(malloc_mem);
      cur = (malloc_mem *)((unsigned long)cur + cur->size + sizeof(malloc_mem));
      if(cur != (malloc_mem *)(page_list->index * 0x1000 + top + page_list->total))
        cur->previous = pre->size;
    }
  }
}

void memory_reserve(unsigned long start, unsigned long end){
  int start_index = (start-(unsigned long)top)/0x1000;
  int end_index = (end-(unsigned long)top)/0x1000;
  // int start_index = start;
  // int end_index = end;

  int head_index = start_index ;
  while (frame[head_index] == -1){
    head_index--;
  }

  while(head_index != start_index){
    delete_frame_list(frame[head_index], head_index);
    frame[head_index] = frame[head_index]-1;
    add_frmae_list(frame[head_index], head_index);
    frame[head_index+power_of_two(frame[head_index])] = frame[head_index];
    add_frmae_list(frame[head_index], head_index+power_of_two(frame[head_index]));
    head_index += power_of_two(frame[head_index]);
    if(head_index > start_index){
      head_index -= power_of_two(frame[head_index]);
    }
  }
  while(head_index+power_of_two(frame[head_index]) < end_index){
    delete_frame_list(frame[head_index], head_index);
    for(int i=1; i<power_of_two(frame[head_index]); i++)
      frame[head_index+i] = PAGE_RESERVED;
    head_index += power_of_two(frame[head_index]);
  }
  while(head_index+power_of_two(frame[head_index])-1 != end_index){
    delete_frame_list(frame[head_index], head_index);
    frame[head_index] = frame[head_index]-1;
    add_frmae_list(frame[head_index], head_index);
    frame[head_index+power_of_two(frame[head_index])] = frame[head_index];
    add_frmae_list(frame[head_index], head_index+power_of_two(frame[head_index]));
    if(end_index >= head_index+power_of_two(frame[head_index])){
      delete_frame_list(frame[head_index], head_index);
      for(int i=1; i<power_of_two(frame[head_index]); i++)
        frame[head_index+i] = PAGE_RESERVED;
      head_index += power_of_two(frame[head_index]);
    }
  }
  delete_frame_list(frame[head_index], head_index);
  for(int i=1; i<power_of_two(frame[head_index]); i++)
    frame[head_index+i] = PAGE_RESERVED;
  // print_frame_state();
  // print_free_frame_list();
}

void alloc_page_to_mem(int request){
  int queue_index = dequeue_page_allocate();
  allocated_page *page = &page_allocated[queue_index];
  page->index = page_allocate(request);
  page->total = request;
  page->free = request - sizeof(malloc_mem);
  page->next = NULL;
  malloc_mem *page_init = (malloc_mem *)(page->index*request + top);
  page_init->allocted = 0;
  page_init->previous = 0;
  page_init->size = request - sizeof(malloc_mem);
  if(!mem_alloc_page){
    mem_alloc_page = page;
    return;
  }
  allocated_page *cur = mem_alloc_page;
  while (cur->next){
    cur = cur->next;
  }
  cur->next = page;
}


void enqueue_page_allocate(int value){
  if((free_queue_front+1)%PAGE_MAX_ENTRY == free_queue_end)
    printf("queue is full\n\r");
  else{
    if(free_queue_end == -1)
      free_queue_end = 0;
    free_queue_front += 1;
    if(free_queue_front >= PAGE_MAX_ENTRY && free_queue_end != 0)
      free_queue_front = 0;
    free_page_queue[free_queue_front] = value;
  }
}

int dequeue_page_allocate(){
  int ans = free_page_queue[free_queue_end];
  if(free_queue_front == free_queue_end){
    free_queue_front = -1;
    free_queue_end = -1;
    return -1;
  }else{
    free_queue_end += 1;
    if(free_queue_end == PAGE_MAX_ENTRY)
      free_queue_end = 0;
    return ans;
  }
}

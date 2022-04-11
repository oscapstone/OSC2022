#include "malloc.h"
#include "printf.h"


extern unsigned char _heap_start;
static char* top = (char*) &_heap_start;

void* simple_malloc(size_t size) {
  char* r = top + 0x10;  // reserve for header
  size = 0x10 + size - size%0x10;  // ALIGN 16
  ((malloc_header*)top)->chunk = size;
  top += size + 0x10;
  return r;
}

int frame[PAGE_MAX_ENTRY] = {0}; // frame array for record the status of page

frame_list free_item[MAX_FREE_ITEM];
int free_item_queue[MAX_FREE_ITEM];
int queue_front = -1;
int queue_end = -1;

static frame_list *free_frame_list[6] = {NULL}; // linked-list for free page

static void enqueue_free_item(int value);
static int dequeue_free_item();

static void add_frmae_list(int value, int index);
static int allocate_frame_list(int value);
static void delete_frame_list(int value, int index);

static void page_devide(int value);
static int find_in_free_frame_list(int level, int index);
static void merge(int index);

// function for calculating power of 2 and log of 2
static int get_log_of_two(int value);
static int power_of_two(int value);

void page_init(){

  // init the free queue array for managing the free_item
  for(int i=0; i<PAGE_MAX_ENTRY; i++)
    enqueue_free_item(i);

  int non_init = PAGE_MAX_ENTRY;
  for(int start=0; start<PAGE_MAX_ENTRY;){
    frame[start] = get_log_of_two(non_init);
    add_frmae_list(get_log_of_two(non_init), start);
    start += power_of_two(get_log_of_two(non_init));
    for(int i=PAGE_MAX_ENTRY-non_init+1; i<start; i++)
      frame[i] = PAGE_NOT_ALLOCATE;
    non_init -= power_of_two(get_log_of_two(non_init));
  }
  print_frame_state();
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
  frame_list *free_frame = &free_item[queue_index];
  free_frame->index = index;
  free_frame->queue_index = queue_index;
  free_frame->next = NULL;
  if(value<6 && value >=0){
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
  if(value < 6 && value >= 0){
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
    if(level < 5)
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
  // printf("size: %d, index: %d\n\r", size, index);
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
  for(int i = 0;;i++){
    if(ans > value)
      return --i;
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
  for(int i=0; i<6; i++){
    pre = free_frame_list[i];
    printf("list%d: ", i);
    while (pre){
      printf("%d ", pre->index);
      pre = pre->next;
    }
    printf("\n\r");
  }
}

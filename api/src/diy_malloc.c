
#include "diy_malloc.h"
#include <stddef.h>
#include "uart.h"

// Simple memory allocation -----------------------------------------
void* simple_malloc(size_t size){
  static char pool[SIMPLE_MALLOC_POOL_SIZE];
  static char *ptr = pool;
  char *temp = ptr;
  if(size < 1){
    uart_printf("Error, not enough of SIMPLE_MALLOC_POOL_SIZE=%d, in simple_malloc()\r\n", SIMPLE_MALLOC_POOL_SIZE);
    return NULL;
  }
  ptr += size;
  return (void*) temp;
}


// Buddy system (page allocation) -----------------------------------
// Ref: https://oscapstone.github.io/labs/overview.html, https://grasslab.github.io/NYCU_Operating_System_Capstone/labs/lab3.html
#define ITEM_COUNT(arr) (sizeof(arr) / sizeof(*arr))
#define PAGE_SIZE 4096 // 4kB

#define MAX_CONTI_ALLOCATION_EXPO 6 // i.g. =6 means max allocation size is 4kB * 2^6
#define FRAME_ARRAY_F -1            // The idx’th frame is free, but it belongs to a larger contiguous memory block. Hence, buddy system doesn’t directly allocate it.
#define FRAME_ARRAY_X -2            // The idx’th frame is already allocated, hence not allocatable.
#define FRAME_ARRAY_P -3            // The idx’th frame is preserved, not allocatable
static int *the_frame_array;        // Has the size of (heap_size/PAGE_SIZE), i.e., total_pages
static size_t total_pages = 0;      // = heal_size / PAGE_SIZE

// frame_freelist_arr[i] points to the head of the linked list of free 4kB*(2^i) pages
freeframe_node *frame_freelist_arr[MAX_CONTI_ALLOCATION_EXPO + 1] = {NULL};

// freeframe_node fifo, since
static freeframe_node **freeframe_node_fifo;  // +1 to ease edge condition
static int fifo_in = 0, fifo_out = 0;         // _in == _out: empty
static int fifo_size = 0;
// Return a node from freeframe_node_fifo
static freeframe_node *malloc_freeframe_node(){
  freeframe_node *temp;
  if(fifo_in == fifo_out){
    uart_printf("Exception, no enough space, fifo empty, fifo_in=fifo_out=%d, in malloc_freeframe_node()\r\n", fifo_in);
    return NULL;
  }
  temp = freeframe_node_fifo[fifo_out++];
  if(fifo_out >= fifo_size)
    fifo_out = 0;
  return temp;
}
// Insert a node into freeframe_node_fifo
static void free_freeframe_node(freeframe_node *node){
  freeframe_node_fifo[fifo_in++] = node;
  if(fifo_in >= fifo_size)
    fifo_in = 0;
}

static int log2_floor(uint64_t x){
  int expo = 0;
  while(x != 0x01 && x != 0x00){
    x = x >> 1;
    expo++;
  }
  if(x == 0x01)
    return expo;
  else // x == 0x00
    return -1;
}

// Dump funcs
static void dump_the_frame_array(){
  uart_printf("the_frame_array[] = ");
  for(int i=0; i<total_pages; i++)  uart_printf("%2d ", the_frame_array[i]);
  uart_printf("\r\n");
}
static void dupmp_frame_freelist_arr(){
  uart_printf("frame_freelist_arr[] = \r\n");
  for(int i=MAX_CONTI_ALLOCATION_EXPO; i>=0; i--){
    freeframe_node *node = frame_freelist_arr[i];
    uart_printf("\t4kB *%3d: ", 1 << i);
    while(node != NULL){
      uart_printf("%2d-->", node->index);
      node = node->next;
    }
    uart_printf("NULL\r\n");
  }
}

int alloc_page(int page_cnt){
  int curr_page_cnt = 0;
  int page_allocated = -1;
  freeframe_node *temp_node = NULL;
  freeframe_node **head = NULL;
  // Return if page_cnt is too big
  if(page_cnt > (1 << MAX_CONTI_ALLOCATION_EXPO)){
    uart_printf("Error, cannot allocate contiguous page_cnt=%d, maximum contiguous page=%d\r\n", page_cnt, (1 << MAX_CONTI_ALLOCATION_EXPO));
    return -1;
  }
  // Roud up page_cnt to 2, 4, 8, 16...
  for(int i=0; i<=MAX_CONTI_ALLOCATION_EXPO; i++){
    if(page_cnt <= (1 << i)){
      page_cnt = 1 << i;
      break;
    }
  }

  // Search for free block that fits page_cnt in frame_freelist_arr
  for(int i=0; i<=MAX_CONTI_ALLOCATION_EXPO; i++){
    head = &frame_freelist_arr[i];  // head points to the linked list of sets of same # free pages
    curr_page_cnt = 1 << i;         // # of free pages now head points to
    if(page_cnt <= curr_page_cnt && (*head) != NULL){
      break;
    }
  }

  // Free block found
  if(page_cnt <= curr_page_cnt && (*head) != NULL) {
    page_allocated = (*head)->index;   // Required page#
    // Remove the block found from (*head)
    temp_node = (*head);
    (*head) = (*head)->next;
    free_freeframe_node(temp_node);

    // Mark part of the free block allocated in the_frame_array
    the_frame_array[page_allocated] = page_cnt;
    const int end_idx = page_allocated + page_cnt;
    const int start_idx = page_allocated + 1;
    for(int k=start_idx; k<end_idx; k++)  the_frame_array[k] = FRAME_ARRAY_X;

    // Re-assign the rest of the free block to new buddies
    if(page_cnt < curr_page_cnt) {
      int pages_remain = curr_page_cnt - page_cnt; // pages count remained in the buddy
      int nfblock_size = page_cnt;  // new frame block size, unit=page
      int nfblock_start = end_idx;  // new frame block starting index
      while(pages_remain > 0){
        int fflist_arr_idx = log2_floor(nfblock_size);
        // Insert 1 node into the linked list
        head = &frame_freelist_arr[fflist_arr_idx];  // head points to the linked list of sets of same # free pages
        temp_node = malloc_freeframe_node();
        temp_node->index = nfblock_start;
        temp_node->next = (*head);
        (*head) = temp_node;

        // Mark buddy begining
        the_frame_array[nfblock_start] = nfblock_size;
        nfblock_start += nfblock_size;

        // Calculate the remaining un-reassigned pages
        pages_remain = pages_remain - nfblock_size;
        nfblock_size = nfblock_size << 1;
      }
    }
  }
  // Free block not found
  else {
    uart_printf("Error, not enough of pages. Required %d contiguous pages\r\n", page_cnt);
  }

  dump_the_frame_array();
  dupmp_frame_freelist_arr();
  return page_allocated;
}

/** Merge frame_freelist_arr[fflists_idx] and it's buddy into frame_freelist_arr[fflists_idx+1]. 
 * So before calling this function, a free node should be inserted to the head (frame_freelist_arr[fflists_idx]). Then 
 * call this function. This function search head's buddy. If buddy found, merge them into a larger block, i.e., insert 
 * it(2 nodes become 1) into the head of the linked list of larger block size. Finally return fflists_idx+1.
 * @param fflists_idx: The index of frame_freelist_arr, indicating which block size to merge
 * @return fflists_idx+1 if buddy found. 1 << (fflists_idx+1) is the block size that merged into. 
 *  Return -1 if no buddy to merge.
 *  Return -2 if exception.
*/
static int free_page_merge(int fflists_idx){
  // Return if no head in the linked list
  if(frame_freelist_arr[fflists_idx] == NULL){
    uart_printf("Exception, frame_freelist_arr[%d] is NULL. @line=%d, file:%s\r\n", fflists_idx, __LINE__, __FILE__);
    return -2;
  }
  
  int freeing_page = frame_freelist_arr[fflists_idx]->index;
  int block_size = the_frame_array[freeing_page];   // How many contiguous pages to free
  int buddy_LR = (freeing_page / block_size % 2);   // 0 for left(lower) buddy, 1 for right(higher) buddy
  // Compute the index of the neighbor to merge
  int buddy_page = freeing_page + (buddy_LR ? -block_size : +block_size);
  if(buddy_LR != 0 && buddy_LR != 1){  // Exception
    uart_printf("Exception, shouldn't get here. buddy_LR=%d, @line=%d, file:%s\r\n", buddy_LR, __LINE__, __FILE__);
    return -2;
  }
  
  // Merge head and it's buddy in the linked list
  freeframe_node **head = &frame_freelist_arr[fflists_idx];
  freeframe_node *node = *head;
  freeframe_node *node_prev = NULL;
  fflists_idx++;
  if(fflists_idx < ITEM_COUNT(frame_freelist_arr)){  // Check if larger block is acceptable
    // Search for head's buddy to merge
    while(node != NULL){
      // Merge-able neighbor found, move the node to the linked list of a greater block size
      if(node->index == buddy_page){
        // Remove the node from the linked list
        if(node_prev != NULL){  // Point previous node's next to current node's next, and remove and free head
          node_prev->next = node->next;
          // Remove and free head
          freeframe_node *node_temp = *head;
          *head = (*head)->next;
          free_freeframe_node(node_temp);
        }
        else{                   // No previous node, that is, head is the buddy
          uart_printf("Exception, should not get here, in free_page_merge(), @line=%d, file:%s\r\n", __LINE__, __FILE__);
        }
        
        // Insert node to the linked list of a greater block size
        node->index = (freeing_page < buddy_page) ? freeing_page : buddy_page; // = min(freeing_page, buddy_page), buddy's head is the smaller one
        node->next = frame_freelist_arr[fflists_idx];
        frame_freelist_arr[fflists_idx] = node;

        // Update the frame array
        block_size = block_size << 1;
        const int start_idx = node->index + 1;
        const int end_idx = node->index + block_size;
        the_frame_array[node->index] = block_size;
        for(int i=start_idx; i < end_idx; i++) the_frame_array[i] = FRAME_ARRAY_F;
        
        uart_printf("Merging %d and %d into %d. merged block_size=%d, start_idx=%d, end_idx=%d\r\n", 
          buddy_page, freeing_page, node->index, block_size, start_idx, end_idx);

        return fflists_idx;
      }
      // Advance node
      node_prev = node;
      node = node->next;
    }
  }

  // We are here because either:
  //  1. Cannot merge into a bigger block because it's already biggest. OR
  //  2. No buddy block found in the linked list
  return -1;
}

/** Free a page allocated from alloc_page()
 * @return 0 on success. -1 on error.
*/
int free_page(int page_index){
  int block_size = the_frame_array[page_index]; // How many contiguous pages to free
  int fflists_idx = log2_floor(block_size);

  // Check if ok to free, return -1 if not ok to free
  if(block_size < 0){
    uart_printf("Error, freeing wrong page. the_frame_array[%d]=%d, the page belongs to a block\r\n", page_index, block_size);
    return -1;
  }
  else if(block_size == 1){
    freeframe_node *head = frame_freelist_arr[fflists_idx];
    // Search for page_index in the linked list
    while(head != NULL){
      if(head->index == page_index){
        uart_printf("Error, freeing wrong page. Page %d is already in free lists. block_size=%d\r\n", page_index, block_size);
        return -1;
      }
      head = head->next;
    }
  }
  else{ // block_size > 1
    if(the_frame_array[page_index + 1] != FRAME_ARRAY_X){
      uart_printf("Error, freeing wrong page. Page %d is already in free lists. block_size=%d\r\n", page_index, block_size);
      return -1;
    }
  }

  // Insert a free node in to frame_freelist_arr[fflists_idx]
  freeframe_node *node = malloc_freeframe_node();
  node->index = page_index;
  node->next = frame_freelist_arr[fflists_idx];
  frame_freelist_arr[fflists_idx] = node;
  // Update the frame array
  const int start_idx = page_index + 1;
  const int end_idx = page_index + block_size;
  for(int i=start_idx; i < end_idx; i++) the_frame_array[i] = FRAME_ARRAY_F;
  
  // Merge iterativly
  int fflists_merge = fflists_idx;
  while(fflists_merge >= 0){
    dupmp_frame_freelist_arr();
    fflists_merge = free_page_merge(fflists_merge);
  }

  dump_the_frame_array();
  dupmp_frame_freelist_arr();
  return 0;
}

void alloc_page_init(uint64_t heap_start, uint64_t heap_end){
  freeframe_node *freeframe_node_pool = NULL;
  const size_t heap_size = heap_end - heap_start;
  total_pages = (heap_size / PAGE_SIZE);
  uart_printf("total_pages=%ld, heap_size=%ld bytes\r\n", total_pages, heap_size);

  // Init: allocate space 
  the_frame_array = (int*) simple_malloc(sizeof(int) * total_pages);
  freeframe_node_pool = (freeframe_node  *) simple_malloc(sizeof(freeframe_node) * total_pages);
  freeframe_node_fifo = (freeframe_node **) simple_malloc(sizeof(freeframe_node) * total_pages + 1); // +1 to ease edge condition
  fifo_size = total_pages + 1;
  for(int i=0; i<total_pages; i++){
    free_freeframe_node(&freeframe_node_pool[i]);
  }

  // Buddy system init
  int left_pages = total_pages;
  int buddy_page_cnt = 1 << MAX_CONTI_ALLOCATION_EXPO;
  int i = MAX_CONTI_ALLOCATION_EXPO;
  int frame_arr_idx = 0;
  while(left_pages > 0 && buddy_page_cnt != 0){
    if(left_pages >= buddy_page_cnt){
      
      // Insert a freeframe_node into the linked list frame_freelist_arr[i]
      freeframe_node *node = malloc_freeframe_node();
      node->next = frame_freelist_arr[i];
      node->index = frame_arr_idx;
      frame_freelist_arr[i] = node;
      
      // Fill the frame array
      the_frame_array[frame_arr_idx] = buddy_page_cnt;
      for(int k=frame_arr_idx+1; k<(frame_arr_idx+buddy_page_cnt); k++){      // Fill value <F> for buddy of frame_arr_idx'th page
        the_frame_array[k] = FRAME_ARRAY_F;
      }
      frame_arr_idx += buddy_page_cnt;

      // Substract left pages
      left_pages -= buddy_page_cnt;
    }
    else{
      buddy_page_cnt = buddy_page_cnt >> 1;
      i--;
    }
  }

  // Dump frame array
  dump_the_frame_array();
  dupmp_frame_freelist_arr();
}


#include "alloc.h"
#include "uart.h"
#include "utils.h"
#include "printf.h"

void buddy_test() {
  print_frame_lists();
  uint64_t size[6] = {
      PAGE_SIZE * 1, PAGE_SIZE * 13, PAGE_SIZE * 16,
      PAGE_SIZE * 2, PAGE_SIZE * 4,  PAGE_SIZE * 8,
  };
  page_frame *frame_ptr[6];
  printf("********** buddy allocation test **********\n");
  for (int i = 0; i < 6; i++) {
    printf("Press any key to continue...");
    char c = uart_getc();
    if (c != '\n') printf("\n");
    frame_ptr[i] = buddy_allocate(size[i]);
    printf("Successfully allocate %lld pages\n", size[i] / PAGE_SIZE);
    // if (c == 'p') 
    print_frame_lists();
  }
  print_frame_lists();
  printf("********** buddy free test **********\n");
  for (int i = 0; i < 6; i++) {
    printf("Press any key to continue...");
    char c = uart_getc();
    if (c != '\n') printf("\n");
    buddy_free(frame_ptr[i]);
    printf("Successfully free %lld pages\n", size[i] / PAGE_SIZE);
    // if (c == 'p') 
    print_frame_lists();
  }
}

void dma_test() {
  print_frame_lists();
  print_dma_list();
  // int *ptr1 = malloc(sizeof(int));
  // int *ptr2 = malloc(sizeof(int) * 1024);
  // int *ptr3 = malloc(sizeof(int));
  // print_dma_list();
  // free(ptr1);
  // print_dma_list();
  // free(ptr2);
  // print_dma_list();
  // free(ptr3);

  uint64_t size[6] = {
      sizeof(int) * 1, sizeof(int) * 8,    sizeof(int) * 2201,
      sizeof(int) * 9, sizeof(int) * 3068, sizeof(int) * 100,
  };
  void *ptr[6];

  printf("********** malloc test **********\n");
  for (int i = 0; i < 6; i++) {
    printf("Press any key to continue...");
    char c = uart_getc();
    if (c != '\n') printf("\n");
    ptr[i] = malloc(size[i]);
printf("Successfully allocate %lld bytes in address %p\n", size[i], ptr[i]);
    // if (c == 'p') {
      print_frame_lists();
      print_dma_list();
    // }
  }
  // print_frame_lists();
  // print_dma_list();
  printf("********** free test **********\n");
  for (int i = 0; i < 6; i++) {
    printf("Press any key to continue...");
    char c = uart_getc();
    if (c != '\n') printf("\n");
    free(ptr[i]);
    printf("Successfully free %lld bytes in address %p\n", size[i], ptr[i]);
    // if (c == 'p') {
      print_frame_lists();
      print_dma_list();
    // }
  }
}

void buddy_init() {
  for (int i = 0; i < MAX_PAGE_NUM; i++) {
    frames[i].id = i;
    frames[i].order = -1;
    frames[i].is_allocated = 0;
    frames[i].addr = PAGE_BASE_ADDR + i * PAGE_SIZE;
    frames[i].next = 0;
  }
  for (int i = 0; i < FRAME_LIST_NUM; i++) {
    free_frame_lists[i] = 0;
    used_frame_lists[i] = 0;
  }
  frames[0].order = MAX_FRAME_ORDER;
  free_frame_lists[MAX_FRAME_ORDER] = &frames[0];
  free_dma_list = 0;
}

page_frame * buddy_allocate(uint64_t size) {
  // printf("Enter size (kb): ");
  // int kb_size = read_i();
  uint64_t page_num = size / PAGE_SIZE;
  if (size % PAGE_SIZE != 0) page_num++;
  page_num = align_up_exp(page_num);
  uint64_t order = log2(page_num);

  for (uint64_t i = order; i <= MAX_FRAME_ORDER; i++) {
    // printf("i = ");
    // uart_int(i);
    // printf("\n");

    if (free_frame_lists[i]) {
      int cur_id = free_frame_lists[i]->id;
      free_frame_lists[i] = free_frame_lists[i]->next;
      frames[cur_id].order = order;
      frames[cur_id].is_allocated = 1;
      frames[cur_id].next = used_frame_lists[order];
      used_frame_lists[order] = &frames[cur_id];
      printf("allocate frame index %d (4K x 2^%lld = %lld KB)\n", cur_id, order,
             1 << (order + 2));

      // release redundant memory block
      for (; i > order; i--) {
        int id = cur_id + (1 << (i - 1));
        frames[id].order = i - 1;
        frames[id].is_allocated = 0;
        frames[id].next = free_frame_lists[i - 1];
        free_frame_lists[i - 1] = &frames[id];
        printf(
            "put frame index %d back to free lists (4K x 2^%lld = %lld KB)\n",
            id, frames[id].order, 1 << (frames[id].order + 2));
      }
      printf("\n");
      // print_frame_lists();
      return &frames[cur_id];
    }
  }
  return 0;
}

void buddy_free(page_frame *frame) {
  // printf("Enter address (hex): ");
  // uint64_t addr = read_h();
  uint64_t index = frame->id;
  if (!frames[index].is_allocated) {
    printf("Error: it is already free\n");
    return;
  }
  uint64_t order = frames[index].order;
  // printf("index :");
  // uart_int(index);
  // printf("order :");
  // uart_int(order);
  // printf("\n");
  buddy_unlink(index, 1);
  // iterative merge
  while (order <= MAX_FRAME_ORDER) {
    uint64_t target_index = index ^ (1 << order); // XOR to find same size page
    printf("target_index :");
    uart_int(target_index);
    printf("\n");
    if ((target_index >= MAX_PAGE_NUM) || frames[target_index].is_allocated ||
        (frames[target_index].order != order))
      break;

    printf("merge with frame index %d (4K x 2^%lld = %lld KB)\n", target_index,
           frames[target_index].order, 1 << (frames[target_index].order + 2));
    order += 1;
    // iterative part
    if (index > target_index) index = target_index;
  }
  frames[index].order = order;
  frames[index].next = free_frame_lists[order];
  free_frame_lists[order] = &frames[index];
  printf("put frame index %d back (4K x 2^%lld = %lld KB)\n", index,
         frames[index].order, 1 << (frames[index].order + 2));
  // print_frame_lists();
}

void buddy_unlink(int index, int type) {
  uint64_t order = frames[index].order;
  frames[index].order = -1;
  frames[index].is_allocated = 0;

  if (type == 0) {
    if (free_frame_lists[order] == &frames[index]) {
      free_frame_lists[order] = frames[index].next;
      frames[index].next = 0;
    } else {
      printf("=========linked list search free=======\n");
      for (page_frame *cur = free_frame_lists[order]; cur; cur = cur->next) {
        if (cur->next == &frames[index]) {
          cur->next = frames[index].next;
          frames[index].next = 0;
          break;
        }
      }
    }
  }
  if (type == 1) {
    if (used_frame_lists[order] == &frames[index]) {
      used_frame_lists[order] = frames[index].next;
      frames[index].next = 0;
    } else {
      printf("=========linked list search used=======\n");
      for (page_frame *cur = used_frame_lists[order]; cur; cur = cur->next) {
        if (cur->next == &frames[index]) {
          cur->next = frames[index].next;
          frames[index].next = 0;
          break;
        }
      }
    }
  }
}

void print_frame_lists() {
  printf("========================\n");
  printf("Free frame lists: \n");
  for (int i = MAX_FRAME_ORDER; i >= 0; i--) {
    printf("4K x 2^%d (%d KB):", i, 1 << (i + 2));
    for (page_frame *cur = free_frame_lists[i]; cur; cur = cur->next) {
      printf("  index %d(0x%x)", cur->id, cur->addr);
    }
    printf("\n");
  }
  printf("\n");
  printf("Used frame lists: \n");
  for (int i = MAX_FRAME_ORDER; i >= 0; i--) {
    printf("4K x 2^%d (%d KB):", i, 1 << (i + 2));
    for (page_frame *cur = used_frame_lists[i]; cur; cur = cur->next) {
      printf("  index %d(0x%x)", cur->id, cur->addr);
    }
    printf("\n");
  }
  printf("========================\n");
}

void *malloc(uint64_t size) {
  dma_header *free_slot = 0;
  uint64_t min_size = ((uint64_t)1) << 63;
  // printf("min_size :");
  // uart_int(min_size);
  // printf("\n");
  // printf("size :");
  // uart_int(size);
  // printf("\n");
  // find the smallest free slot which is bigger than the required size
  for (dma_header *cur = free_dma_list; cur; cur = cur->next) {
    // printf("find the smallest free slot which is bigger than the required size\n");

    uint64_t data_size = cur->total_size - align_up(sizeof(dma_header), 8);
    if (data_size >= align_up(size, 8) && data_size < min_size) {
      free_slot = cur;
      min_size = data_size;
    }
  }

  uint64_t allocated_size = align_up(sizeof(dma_header), 8) + align_up(size, 8);
  // printf("allocated_size :");
  // uart_int(allocated_size);
  // printf("\n");
  if (free_slot) {
    printf("have free_slot\n");

    uint64_t addr = (uint64_t)free_slot;
    uint64_t total_size = free_slot->total_size;
    // rewrite the found free slot
    free_slot->total_size = allocated_size;
    free_slot->used_size = size;
    free_slot->is_allocated = 1;
    if (free_slot->prev) {
      // printf("if (free_slot->prev)\n");
      free_slot->prev->next = free_slot->next;
    }
    if (free_slot->next) {
      // printf("if (free_slot->next)\n");
      free_slot->next->prev = free_slot->prev;
    }
    if (free_dma_list == free_slot){
      // printf("if (free_slot->prev)\n");
      free_dma_list = free_slot->next;
    }
    free_slot->prev = 0;
    free_slot->next = 0;

    // create another free slot if remaining size is big enough
    uint64_t free_size =
        total_size - allocated_size - align_up(sizeof(dma_header), 8);
    if (free_size > 0) {
      dma_header *new_header = (dma_header *)(addr + allocated_size);
      new_header->total_size = total_size - allocated_size;
      new_header->used_size = 0;
      new_header->is_allocated = 0;
      new_header->frame_ptr = free_slot->frame_ptr;
      new_header->prev = 0;
      new_header->next = free_dma_list;
      if (free_dma_list) free_dma_list->prev = new_header;
      free_dma_list = new_header;
    } else {
      free_slot->total_size = total_size;
    }
    return (void *)(addr + align_up(sizeof(dma_header), 8));
  } else {
    printf("no free_slot\n");

    // allocate a page
    page_frame *frame_ptr = buddy_allocate(allocated_size);
    uint64_t addr = frame_ptr->addr;
    // printf("addr :");
    // uart_hex(addr);
    // printf("\n");
    // create a free slot
    dma_header *allocated_header = (dma_header *)addr;
    allocated_header->total_size = allocated_size;
    allocated_header->used_size = size;
    allocated_header->is_allocated = 1;
    allocated_header->frame_ptr = frame_ptr;
    allocated_header->prev = 0;
    allocated_header->next = 0;
    // create another free slot if remaining size is big enough
    uint64_t order = frame_ptr->order;
    uint64_t total_size = (1 << order) * 4 * kb;
    uint64_t free_size = total_size - allocated_size - align_up(sizeof(dma_header), 8);
    // printf("order :");
    // uart_int(order);
    // printf("\n");
    // printf("total_size :");
    // uart_int(total_size);
    // printf("\n");
    // printf("free_size :");
    // uart_int(free_size);
    // printf("\n");
    if (free_size > 0) {
      dma_header *new_header = (dma_header *)(addr + allocated_size);
      new_header->total_size = total_size - allocated_size;
      new_header->used_size = 0;
      new_header->is_allocated = 0;
      new_header->frame_ptr = frame_ptr;
      new_header->prev = 0;
      new_header->next = free_dma_list;
      if (free_dma_list) free_dma_list->prev = new_header;
      free_dma_list = new_header;
    } else {
      allocated_header->total_size = total_size;
    }
    return (void *)(addr + align_up(sizeof(dma_header), 8));
  }
  return 0;
}

void free(void *ptr) {
  uint64_t target_addr = (uint64_t)ptr - align_up(sizeof(dma_header), 8);
  dma_header *target_header = (dma_header *)target_addr;
  target_header->used_size = 0;
  target_header->is_allocated = 0;
  target_header->prev = 0;
  target_header->next = free_dma_list;
  if (free_dma_list) free_dma_list->prev = target_header;
  free_dma_list = target_header;

  // uart_hex((uint64_t)ptr);
  // printf("\n");
  // uart_hex(target_addr);
  // printf("\n");

  page_frame *frame_ptr = target_header->frame_ptr;
  uint64_t base_addr = frame_ptr->addr;
  uint64_t order = frame_ptr->order;
  uint64_t total_frame_size = (1 << order) * 4 * kb;
  uint64_t boundary = base_addr + total_frame_size;

  // uart_hex(base_addr);
  // printf("\n");
  // uart_hex(total_size);
  // printf("\n");

  // merge next slot if it is free
  uint64_t next_addr = target_addr + target_header->total_size;
  dma_header *next_header = (dma_header *)next_addr;
  if (next_addr < boundary && !next_header->is_allocated) {
    if (next_header->prev) next_header->prev->next = next_header->next;
    if (next_header->next) next_header->next->prev = next_header->prev;
    if (free_dma_list == next_header) free_dma_list = next_header->next;
    next_header->prev = 0;
    next_header->next = 0;
    target_header->total_size += next_header->total_size;
  }
  // uart_int(target_header->total_size);
  // printf("\n");
  // print_dma_list();

  // merge previous slot if it is free
  uint64_t current_addr = base_addr;
  while (current_addr < boundary) {
    dma_header *header = (dma_header *)current_addr;
    uint64_t next_addr = current_addr + header->total_size;
    // uart_hex(current_addr);
    // printf("\n");
    // uart_hex(next_addr);
    // printf("\n");
    if (next_addr == target_addr) {
      if (!header->is_allocated) {
        header->total_size += target_header->total_size;
        // uart_int(header->total_size);
        // printf("\n");
        if (target_header->prev)
          target_header->prev->next = target_header->next;
        if (target_header->next)
          target_header->next->prev = target_header->prev;
        if (free_dma_list == target_header) free_dma_list = target_header->next;
        target_header->prev = 0;
        target_header->next = 0;
      }
      break;
    }
    current_addr = next_addr;
  }

  // free page frame if all slots are free
  dma_header *base_header = (dma_header *)base_addr;
  if (base_header->total_size == total_frame_size) {
    if (base_header->prev) base_header->prev->next = base_header->next;
    if (base_header->next) base_header->next->prev = base_header->prev;
    if (free_dma_list == base_header) free_dma_list = base_header->next;
    base_header->prev = 0;
    base_header->next = 0;
    buddy_free(frame_ptr);
  }
}

void print_dma_list() {
  printf("========================\n");
  printf("Free DMA slots: \n");
  for (dma_header *cur = free_dma_list; cur; cur = cur->next) {
    printf("size: ");
    uart_int(cur->total_size - align_up(sizeof(dma_header), 8));
    printf(", frame index: ");
    uart_int(cur->frame_ptr->id);
    printf("\n");
  }
  printf("========================\n");
}

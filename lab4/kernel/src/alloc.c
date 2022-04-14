#include "alloc.h"
#include "uart.h"
#include "utils.h"

void buddy_test() {
  print_frame_lists();
  uint64_t size[6] = {
      PAGE_SIZE * 1, PAGE_SIZE * 13, PAGE_SIZE * 16,
      PAGE_SIZE * 2, PAGE_SIZE * 4,  PAGE_SIZE * 8,
  };
  page_frame *frame_ptr[6];
  uart_puts("********** buddy allocation test **********\n");
  for (int i = 0; i < 6; i++) {
    uart_puts("Press any key to continue...");
    char c = uart_getc();
    if (c != '\n') uart_puts("\n");
    frame_ptr[i] = buddy_allocate(size[i]);
    uart_puts("Successfully allocate ");
    uart_int(size[i] / PAGE_SIZE);
    uart_puts(" pages\n");
    // if (c == 'p') 
    print_frame_lists();
  }
  print_frame_lists();
  uart_puts("********** buddy free test **********\n");
  for (int i = 0; i < 6; i++) {
    uart_puts("Press any key to continue...");
    char c = uart_getc();
    if (c != '\n') uart_puts("\n");
    buddy_free(frame_ptr[i]);
    uart_puts("Successfully free ");
    uart_int(size[i] / PAGE_SIZE);
    uart_puts(" pages\n");
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

  uart_puts("********** malloc test **********\n");
  for (int i = 0; i < 6; i++) {
    uart_puts("Press any key to continue...");
    char c = uart_getc();
    if (c != '\n') uart_puts("\n");
    ptr[i] = malloc(size[i]);
    uart_puts("Successfully allocate ");
    uart_int(size[i]);
    uart_puts(" bytes in address ");
    uart_hex((uint64_t)ptr[i]);
    uart_puts("\n");
    // if (c == 'p') {
      print_frame_lists();
      print_dma_list();
    // }
  }
  // print_frame_lists();
  // print_dma_list();
  uart_puts("********** free test **********\n");
  for (int i = 0; i < 6; i++) {
    uart_puts("Press any key to continue...");
    char c = uart_getc();
    if (c != '\n') uart_puts("\n");
    free(ptr[i]);
    uart_puts("Successfully free ");
    uart_int(size[i]);
    uart_puts(" bytes in address ");
    uart_hex((uint64_t)ptr[i]);
    uart_puts("\n");
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
  // uart_puts("Enter size (kb): ");
  // int kb_size = read_i();
  uint64_t page_num = size / PAGE_SIZE;
  if (size % PAGE_SIZE != 0) page_num++;
  page_num = align_up_exp(page_num);
  uint64_t order = log2(page_num);

  for (uint64_t i = order; i <= MAX_FRAME_ORDER; i++) {
    // uart_puts("i = ");
    // uart_int(i);
    // uart_puts("\n");

    if (free_frame_lists[i]) {
      int cur_id = free_frame_lists[i]->id;
      free_frame_lists[i] = free_frame_lists[i]->next;
      frames[cur_id].order = order;
      frames[cur_id].is_allocated = 1;
      frames[cur_id].next = used_frame_lists[order];
      used_frame_lists[order] = &frames[cur_id];
      uart_puts("allocate frame index ");
      uart_int(cur_id);
      uart_puts(" (4K x 2^");
      uart_int(order);
      uart_puts(" = ");
      uart_int(1 << (order + 2));
      uart_puts(" KB)\n");

      // release redundant memory block
      for (; i > order; i--) {
        int id = cur_id + (1 << (i - 1));
        frames[id].order = i - 1;
        frames[id].is_allocated = 0;
        frames[id].next = free_frame_lists[i - 1];
        free_frame_lists[i - 1] = &frames[id];
        uart_puts("put frame index ");
        uart_int(id);
        uart_puts(" back to free lists (4K x 2^");
        uart_int(frames[id].order);
        uart_puts(" = ");
        uart_int(1 << (frames[id].order + 2));
        uart_puts(" KB)\n");
      }
      uart_puts("\n");
      // print_frame_lists();
      return &frames[cur_id];
    }
  }
  return 0;
}

void buddy_free(page_frame *frame) {
  // uart_puts("Enter address (hex): ");
  // uint64_t addr = read_h();
  uint64_t index = frame->id;
  if (!frames[index].is_allocated) {
    uart_puts("Error: it is already free\n");
    return;
  }
  uint64_t order = frames[index].order;
  // uart_puts("index :");
  // uart_int(index);
  // uart_puts("order :");
  // uart_int(order);
  // uart_puts("\n");
  buddy_unlink(index, 1);
  // iterative merge
  while (order <= MAX_FRAME_ORDER) {
    uint64_t target_index = index ^ (1 << order); // XOR to find same size page
    uart_puts("target_index :");
    uart_int(target_index);
    uart_puts("\n");
    if ((target_index >= MAX_PAGE_NUM) || frames[target_index].is_allocated ||
        (frames[target_index].order != order))
      break;

    uart_puts("merge with frame index ");
    uart_int(target_index);
    uart_puts(" (4K x 2^");
    uart_int(frames[target_index].order);
    uart_puts(" = ");
    uart_int(1 << (frames[target_index].order + 2));
    uart_puts(" KB)\n");
    buddy_unlink(target_index, 0);
    order += 1;
    // iterative part
    if (index > target_index) index = target_index;
  }
  frames[index].order = order;
  frames[index].next = free_frame_lists[order];
  free_frame_lists[order] = &frames[index];
  uart_puts("put frame index ");
  uart_int(index);
  uart_puts(" back (4K x 2^");
  uart_int(frames[index].order);
  uart_puts(" = ");
  uart_int(1 << (frames[index].order + 2));
  uart_puts(" KB)\n\n");
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
      uart_puts("=========linked list search free=======\n");
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
      uart_puts("=========linked list search used=======\n");
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
  uart_puts("========================\n");
  uart_puts("Free frame lists: \n");
  for (int i = MAX_FRAME_ORDER; i >= 0; i--) {
    uart_puts("4K x 2^");
    uart_int(i);
    uart_puts(" (");
    uart_int(1 << (i + 2));
    uart_puts(" KB):");
    for (page_frame *cur = free_frame_lists[i]; cur; cur = cur->next) {
      uart_puts("  index ");
      uart_int(cur->id);
      uart_puts("(");
      uart_hex(cur->addr);
      uart_puts(")");
    }
    uart_puts("\n");
  }
  uart_puts("\n");
  uart_puts("Used frame lists: \n");
  for (int i = MAX_FRAME_ORDER; i >= 0; i--) {
    uart_puts("4K x 2^");
    uart_int(i);
    uart_puts(" (");
    uart_int(1 << (i + 2));
    uart_puts(" KB):");
    for (page_frame *cur = used_frame_lists[i]; cur; cur = cur->next) {
      uart_puts("  index ");
      uart_int(cur->id);
      uart_puts("(");
      uart_hex(cur->addr);
      uart_puts(")");
    }
    uart_puts("\n");
  }
  uart_puts("========================\n");
}

void *malloc(uint64_t size) {
  dma_header *free_slot = 0;
  uint64_t min_size = ((uint64_t)1) << 63;
  // uart_puts("min_size :");
  // uart_int(min_size);
  // uart_puts("\n");
  // uart_puts("size :");
  // uart_int(size);
  // uart_puts("\n");
  // find the smallest free slot which is bigger than the required size
  for (dma_header *cur = free_dma_list; cur; cur = cur->next) {
    // uart_puts("find the smallest free slot which is bigger than the required size\n");

    uint64_t data_size = cur->total_size - align_up(sizeof(dma_header), 8);
    if (data_size >= align_up(size, 8) && data_size < min_size) {
      free_slot = cur;
      min_size = data_size;
    }
  }

  uint64_t allocated_size = align_up(sizeof(dma_header), 8) + align_up(size, 8);
  // uart_puts("allocated_size :");
  // uart_int(allocated_size);
  // uart_puts("\n");
  if (free_slot) {
    uart_puts("have free_slot\n");

    uint64_t addr = (uint64_t)free_slot;
    uint64_t total_size = free_slot->total_size;
    // rewrite the found free slot
    free_slot->total_size = allocated_size;
    free_slot->used_size = size;
    free_slot->is_allocated = 1;
    if (free_slot->prev) {
      // uart_puts("if (free_slot->prev)\n");
      free_slot->prev->next = free_slot->next;
    }
    if (free_slot->next) {
      // uart_puts("if (free_slot->next)\n");
      free_slot->next->prev = free_slot->prev;
    }
    if (free_dma_list == free_slot){
      // uart_puts("if (free_slot->prev)\n");
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
    uart_puts("no free_slot\n");

    // allocate a page
    page_frame *frame_ptr = buddy_allocate(allocated_size);
    uint64_t addr = frame_ptr->addr;
    // uart_puts("addr :");
    // uart_hex(addr);
    // uart_puts("\n");
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
    // uart_puts("order :");
    // uart_int(order);
    // uart_puts("\n");
    // uart_puts("total_size :");
    // uart_int(total_size);
    // uart_puts("\n");
    // uart_puts("free_size :");
    // uart_int(free_size);
    // uart_puts("\n");
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
  // uart_puts("\n");
  // uart_hex(target_addr);
  // uart_puts("\n");

  page_frame *frame_ptr = target_header->frame_ptr;
  uint64_t base_addr = frame_ptr->addr;
  uint64_t order = frame_ptr->order;
  uint64_t total_frame_size = (1 << order) * 4 * kb;
  uint64_t boundary = base_addr + total_frame_size;

  // uart_hex(base_addr);
  // uart_puts("\n");
  // uart_hex(total_size);
  // uart_puts("\n");

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
  // uart_puts("\n");
  // print_dma_list();

  // merge previous slot if it is free
  uint64_t current_addr = base_addr;
  while (current_addr < boundary) {
    dma_header *header = (dma_header *)current_addr;
    uint64_t next_addr = current_addr + header->total_size;
    // uart_hex(current_addr);
    // uart_puts("\n");
    // uart_hex(next_addr);
    // uart_puts("\n");
    if (next_addr == target_addr) {
      if (!header->is_allocated) {
        header->total_size += target_header->total_size;
        // uart_int(header->total_size);
        // uart_puts("\n");
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
  uart_puts("========================\n");
  uart_puts("Free DMA slots: \n");
  for (dma_header *cur = free_dma_list; cur; cur = cur->next) {
    uart_puts("size: ");
    uart_int(cur->total_size - align_up(sizeof(dma_header), 8));
    uart_puts(", frame index: ");
    uart_int(cur->frame_ptr->id);
    uart_puts("\n");
  }
  uart_puts("========================\n");
}

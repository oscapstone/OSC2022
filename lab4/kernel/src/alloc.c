#include "alloc.h"
#include "uart.h"

void buddy_test() {
  uint64_t size[6] = {
      PAGE_SIZE * 1, PAGE_SIZE * 16, PAGE_SIZE * 16,
      PAGE_SIZE * 2, PAGE_SIZE * 4,  PAGE_SIZE * 8,
  };
  uint64_t addr[6];
  uart_puts("========== allocation test ==========\n");
  for (int i = 0; i < 6; i++) {
    addr[i] = buddy_allocate(size[i]);
    print_frame_lists();
  }
  print_frame_lists();
  uart_puts("========== free test ==========\n");
  for (int i = 0; i < 6; i++) {
    buddy_free(addr[i]);
    print_frame_lists();
  }
  print_frame_lists();
}

void buddy_init() {
  for (int i = 0; i < MAX_PAGE_NUM; i++) {
    frames[i].id = i;
    frames[i].order = -1;
    frames[i].is_allocated = 0;
    frames[i].addr = PAGE_BASE_ADDR + i * PAGE_SIZE;
    frames[i].prev = 0;
    frames[i].next = 0;
  }
  for (int i = 0; i < FRAME_LIST_NUM; i++) {
    free_frame_lists[i] = 0;
    used_frame_lists[i] = 0;
  }
  frames[0].order = MAX_FRAME_ORDER;
  free_frame_lists[MAX_FRAME_ORDER] = &frames[0];
  print_frame_lists();
}

uint64_t buddy_allocate(uint64_t size) {
  // uart_puts("Enter size (kb): ");
  // int kb_size = read_i();
  uint64_t page_num = size / PAGE_SIZE;
  uint64_t order = log2(page_num);

  for (uint64_t i = order; i <= MAX_FRAME_ORDER; i++) {
    uart_puts("i = ");
    uart_int(i);
    uart_puts("\n");

    if (free_frame_lists[i]) {
      int cur_id = free_frame_lists[i]->id;
      free_frame_lists[i] = free_frame_lists[i]->next;
      frames[cur_id].order = order;
      frames[cur_id].is_allocated = 1;
      frames[cur_id].next = used_frame_lists[order];
      used_frame_lists[order] = &frames[cur_id];
      uart_puts("allocate index ");
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
        uart_puts("put index ");
        uart_int(id);
        uart_puts(" back (4K x 2^");
        uart_int(frames[id].order);
        uart_puts(" = ");
        uart_int(1 << (frames[id].order + 2));
        uart_puts(" KB)\n");
      }
      uart_puts("\n");
      // print_frame_lists();
      return frames[cur_id].addr;
    }
  }
  return 0;
}

void buddy_free(uint64_t addr) {
  // uart_puts("Enter address (hex): ");
  // uint64_t addr = read_h();
  uint64_t index = (addr - PAGE_BASE_ADDR) / PAGE_SIZE;
  if (!frames[index].is_allocated) {
    uart_puts("Error: it is already free\n");
    return;
  }
  uint64_t order = frames[index].order;
  uart_puts("index :");
  uart_int(index);
  uart_puts("order :");
  uart_int(order);
  uart_puts("\n");
  unlink(index, 1);
  // iterative merge
  while (order <= MAX_FRAME_ORDER) {
    uint64_t target_index = index ^ (1 << order); // XOR to find same size page
    uart_puts("target_index :");
    uart_int(target_index);
    uart_puts("\n");
    if ((target_index >= MAX_PAGE_NUM) || frames[target_index].is_allocated ||
        (frames[target_index].order != order))
      break;

    uart_puts("merge with index ");
    uart_int(target_index);
    uart_puts(" (4K x 2^");
    uart_int(frames[target_index].order);
    uart_puts(" = ");
    uart_int(1 << (frames[target_index].order + 2));
    uart_puts(" KB)\n");
    unlink(target_index, 0);
    order += 1;
    // iterative part
    if (index > target_index) index = target_index;
  }
  frames[index].order = order;
  frames[index].next = free_frame_lists[order];
  free_frame_lists[order] = &frames[index];
  uart_puts("put index ");
  uart_int(index);
  uart_puts(" back (4K x 2^");
  uart_int(frames[index].order);
  uart_puts(" = ");
  uart_int(1 << (frames[index].order + 2));
  uart_puts(" KB)\n\n");
  // print_frame_lists();
}

void unlink(int index, int type) {
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
  uart_puts("Free lists: \n");
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
  uart_puts("Used lists: \n");
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
  uart_puts("\n");
}
#include "uart.h"
#include "utils.h"

uint64_t allocate_end = 0x6000000;
void* malloc(int size)
{
    uint64_t location = allocate_end;
    allocate_end += size;
    // uart_puts("allocate memory at:");
    // uart_hex(location);
    // uart_puts(", with size:");
    // uart_hex(size);
    // uart_puts("\r\n");

    return (void *)location;
}
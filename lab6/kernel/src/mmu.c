#include "mmu.h"

#include <stdint.h>
#include "alloc.h"
#include "printf.h"
#include "thread.h"
#include "exception.h"
#include "mbox.h"

void init_page_table(thread_info *thread, uint64_t **table) {
  *table = (uint64_t *)thread_allocate_page(thread, PAGE_SIZE);
  for (int i = 0; i < 512; i++) {
    *((*table) + i) = 0;
  }
  // printf("[init] next table virtual addr: 0x%llx\n", (uint64_t)(*table));
  *table = (uint64_t *)VA2PA(*table);
}

void update_page_table(thread_info *thread, uint64_t virtual_addr,
                       uint64_t physical_addr, uint64_t flags) {
  if (thread->pgd == 0) {
    printf("Invalid PGD!!\n");
    return;
  }

  uint32_t index[4] = {
      (virtual_addr >> 39) & 0x1ff, (virtual_addr >> 30) & 0x1ff,
      (virtual_addr >> 21) & 0x1ff, (virtual_addr >> 12) & 0x1ff};

  // printf("virtual addr: 0x%llx", virtual_addr);
  // printf(", index: 0x%llx", index[0]);
  // printf(", index: 0x%llx", index[1]);
  // printf(", index: 0x%llx", index[2]);
  // printf(", index: 0x%llx\n", index[3]);
  // printf("physical addr: 0x%llx\n", physical_addr);

  uint64_t *table = (uint64_t *)PA2VA(thread->pgd);
  // printf("table: 0x%llx\n", (uint64_t)table);
  for (int level = 0; level < 3; level++) {
    if (table[index[level]] == 0) {
      // printf("level: %d, index: 0x%llx  ", level, index[level]);
      init_page_table(thread, (uint64_t **)&(table[index[level]]));
      table[index[level]] |= PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_TABLE;
    }
    // printf("table PA: 0x%llx\n", (uint64_t)table[index[level]]);
    table = (uint64_t *)PA2VA(table[index[level]] & ~0xfff);
    // printf("table VA: 0x%llx\n", (uint64_t)table);
  }
  table[index[3]] =
      physical_addr | BOOT_PTE_NORMAL_NOCACHE_ATTR | flags;
  // printf("page PA: 0x%llx\n", (uint64_t)table[index[3]]);
}

uint64_t el0_VA2PA(thread_info *thread, uint64_t virtual_addr) {
  disable_interrupt();
  uint32_t index[4] = {
      (virtual_addr >> 39) & 0x1ff, (virtual_addr >> 30) & 0x1ff,
      (virtual_addr >> 21) & 0x1ff, (virtual_addr >> 12) & 0x1ff};
  uint32_t offset = virtual_addr &0xfff;
  // printf("offset =%p\n",offset);
  printf("[el0_VA2PA]virtual addr: %p\n", virtual_addr);
  // printf(", index: 0x%llx", index[0]);
  // printf(", index: 0x%llx", index[1]);
  // printf(", index: 0x%llx", index[2]);
  // printf(", index: 0x%llx\n", index[3]);

  uint64_t *table = (uint64_t *)PA2VA(thread->pgd);
  // printf("pgd: %p\n", (uint64_t)table);
  for (int level = 0; level < 3; level++) {
    if (table[index[level]] == 0) {
      printf("[el0_VA2PA]*Your page table implement unsuccessful*\n");
      break;
    }
    // printf("  [el0_VA2PA]table PA: 0x%p\n", (uint64_t)table[index[level]]);
    table = (uint64_t *)PA2VA(table[index[level]] & ~0xfff);
    // printf("table VA: 0x%llx\n", (uint64_t)table);
  }
  printf("[el0_VA2PA]transfer to physical addr: %p\n",(table[index[3]]& ~0xfff)|offset);
  return (table[index[3]]& ~0xfff)|offset;
}
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "mmu.h"
#include "mem.h"
#include "task.h"

/* return the virtual address of a pgd */
void* create_pgd(struct taskControlBlock* tsk) {
  if (!tsk->mem.pgd) {
    tsk->mem.pgd = (uint64_t)getFreePage();
    void* new_page = (void*)tsk->mem.pgd + VA_START;
    memset(new_page, 0, FRAME_SIZE);
  }
  return (void*)tsk->mem.pgd + VA_START;
}

/* return the kva of the next level table */
uint64_t* map_table(uint64_t *table, uint64_t shift, uint64_t va, int *new_table) {
  uint64_t idx = va >> shift;
  idx = idx & PT_IDX_MASK;
  if (!table[idx]) {
    *new_table = 1;
    uint64_t nxt_table = (uint64_t)getFreePage();
    table[idx] = nxt_table | PD_TABLE;
    return (uint64_t*)(nxt_table + VA_START);
  }
  *new_table = 0;
  return (uint64_t*)((table[idx] & PAGE_MASK) + VA_START);
}

void map_entry(uint64_t *pte, uint64_t va, uint64_t pa) {
  uint64_t idx = (va >> PTE_SHIFT) & PT_IDX_MASK;
  pte[idx] = pa | NORM_PTE_ATTR;
}

void map_page(struct taskControlBlock* tsk, uint64_t va, uint64_t pa) {
  uint64_t* pgd = create_pgd(tsk);
  int new_table = 0;
  uint64_t* pud = map_table(pgd, PGD_SHIFT, va, &new_table);
  uint64_t* pmd = map_table(pud, PUD_SHIFT, va, &new_table);
  uint64_t* pte = map_table(pmd, PMD_SHIFT, va, &new_table);
  map_entry(pte, va, pa);
}

void* allocate_user_page(struct taskControlBlock* tsk, uint64_t va, int page_num) {
  int exp;
  uint64_t pa_start = (uint64_t)getContFreePage(page_num, &exp);
  if (pa_start == 0) return NULL;
  int real_cnt = 1<<exp;
  uint64_t cur_va = va, cur_pa = pa_start;
  for (int i = 0; i < real_cnt; i++) {
    map_page(tsk, cur_va, cur_pa);
    cur_va += FRAME_SIZE;
    cur_pa += FRAME_SIZE;
  }
  return (void*)pa_start + VA_START;
}


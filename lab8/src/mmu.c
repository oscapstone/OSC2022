#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <textio.h>
#include "mmu.h"
#include "mem.h"
#include "task.h"


void* allocate_kernel_page(struct taskControlBlock* tsk, int exp) {
  void* kva = getFreePage() + VA_START;
  tsk->mem.kernel_pages[tsk->mem.kernel_pages_cnt].vBegin = kva;
  tsk->mem.kernel_pages[tsk->mem.kernel_pages_cnt].exp = exp;
  tsk->mem.kernel_pages_cnt += 1;
  memset(kva, 0, FRAME_SIZE * (1<<exp));
  return kva;
}

/* return the va of the next level table */
uint64_t* map_table(struct taskControlBlock* tsk, uint64_t *table, uint64_t shift, uint64_t va) {
  uint64_t idx = va >> shift;
  idx = idx & PT_IDX_MASK;
  if (!table[idx]) {
    void* vNxtTbl = allocate_kernel_page(tsk, 0);
    void* pNxtTbl = vNxtTbl - VA_START;
    table[idx] = (uint64_t)pNxtTbl | PD_TABLE;
    return (uint64_t*)(vNxtTbl);
  }
  return (uint64_t*)((table[idx] & PAGE_MASK) + VA_START);
}

void map_entry(uint64_t *pte, uint64_t va, uint64_t pa) {
  uint64_t idx = (va >> PTE_SHIFT) & PT_IDX_MASK;
  pte[idx] = pa | NORM_PTE_ATTR;
}

/* return the virtual address of a pgd */
void* create_pgd(struct taskControlBlock* tsk) {
  if (!tsk->mem.pgd) {
    void* vPGD = allocate_kernel_page(tsk, 0);
    tsk->mem.pgd = (uint64_t)vPGD - VA_START;
    memset(vPGD, 0, FRAME_SIZE);

    flat_mapping_block(tsk, 0x3C000000, 24);
    
  }
  return (void*)tsk->mem.pgd + VA_START;
}

void map_page(struct taskControlBlock* tsk, uint64_t va, uint64_t pa) {
  uint64_t* pgd = create_pgd(tsk);
  uint64_t* pud = map_table(tsk, pgd, PGD_SHIFT, va);
  uint64_t* pmd = map_table(tsk, pud, PUD_SHIFT, va);
  uint64_t* pte = map_table(tsk, pmd, PMD_SHIFT, va);
  map_entry(pte, va, pa);
}

void* allocate_user_page(struct taskControlBlock* tsk, uint64_t va, int page_num) {
  int exp;
  void* pa_start = getContFreePage(page_num, &exp);
  if (pa_start == 0) return NULL;

  tsk->mem.user_pages[tsk->mem.user_pages_cnt].vBegin = (void*)va;
  tsk->mem.user_pages[tsk->mem.user_pages_cnt].exp = exp;
  tsk->mem.user_pages_cnt += 1;
  
  int real_cnt = 1<<exp;
  uint64_t cur_va = va, cur_pa = (uint64_t)pa_start;
  for (int i = 0; i < real_cnt; i++) {
    map_page(tsk, cur_va, cur_pa);
    cur_va += FRAME_SIZE;
    cur_pa += FRAME_SIZE;
  }
  return pa_start + VA_START;
}

void* flat_mapping_block(struct taskControlBlock* tsk, uint64_t addr, uint64_t num_2m_blk) {
  // kprintf("flat mapping: 0x%x~0x%x\n", addr, addr+num_2m_blk*0x1000*512);
  uint64_t offaddr = addr;
  for (int i = 0; i < num_2m_blk; i++) {
    uint64_t* pgd = create_pgd(tsk);
    uint64_t* pud = map_table(tsk, pgd, PGD_SHIFT, offaddr);
    uint64_t* pmd = map_table(tsk, pud, PUD_SHIFT, offaddr);
    uint64_t idx = (offaddr >> PMD_SHIFT) & PT_IDX_MASK;
    pmd[idx] = offaddr | NORM_PDB_ATTR;
    offaddr += FRAME_SIZE * 512;
    // kprintf("addr: %lx, pmd: %lx, idx: %d, val: %lx\n", offaddr, pmd, idx, pmd[idx]);
  }
  return (void*)addr + VA_START;
}

void* translate(struct taskControlBlock* tsk, uint64_t va) {
  uint64_t* pgd = (uint64_t*)(tsk->mem.pgd + VA_START);
  uint64_t* pud = (uint64_t*)((pgd[(va>>PGD_SHIFT) & PT_IDX_MASK] & PAGE_MASK) + VA_START);
  uint64_t* pmd = (uint64_t*)((pud[(va>>PUD_SHIFT) & PT_IDX_MASK] & PAGE_MASK) + VA_START);
  uint64_t* pte = (uint64_t*)((pmd[(va>>PMD_SHIFT) & PT_IDX_MASK] & PAGE_MASK) + VA_START);
  void* pPage = (void*)(pte[(va>>PTE_SHIFT) & PT_IDX_MASK] & PAGE_MASK) + VA_START;
  return pPage + (va & 0xfff);
}

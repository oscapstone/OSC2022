#ifndef MMU_H
#define MMU_H

#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1


#define ATTR_RW (0 << 7)
#define ATTR_RO (1 << 7)

#define PD_TABLE 0b11
#define PD_PAGE 0b11
#define PD_BLOCK 0b01
#define PD_ACCESS (1 << 10)
#define PD_ACCESS_PERM (0x01 << 6)

#define BOOT_PGD_ATTR PD_TABLE
#define BOOT_DEV_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)
#define BOOT_NORM_ATTR (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK)

#define NORM_PTE_ATTR (PD_PAGE | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_ACCESS | PD_ACCESS_PERM)
#define NORM_PDB_ATTR (PD_BLOCK | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_ACCESS | PD_ACCESS_PERM)
#define DEV_PDB_ATTR (PD_BLOCK | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_ACCESS)

#define VA_START 0xffff000000000000
#define NORMAL_ATTR ATTR_RW | PD_ACCESS | PD_TABLE

#define EL1_PGD 0x40000
#define EL1_PUD 0x41000

#define PGD_SHIFT 39
#define PUD_SHIFT 30
#define PMD_SHIFT 21
#define PTE_SHIFT 12

#define PAGE_MASK 0xfffffffffffff000
#define PT_IDX_MASK 0x1ff
#define PT_ENTRY_NUM 512

#ifndef __ASSEMBLER__
#include <stdint.h>
#include "task.h"

/* return the virtual address of a pgd */
void* create_pgd(struct taskControlBlock* tsk);
/* return the kva of the next level table */
uint64_t* map_table(struct taskControlBlock* tsk, uint64_t *table, uint64_t shift, uint64_t va);
void map_entry(uint64_t *pte, uint64_t va, uint64_t pa);
void map_page(struct taskControlBlock* tsk, uint64_t va, uint64_t pa);
void *allocate_user_page(struct taskControlBlock *tsk, uint64_t va,
                         int page_num);
void *allocate_kernel_page(struct taskControlBlock *tsk, int exp);
void *translate(struct taskControlBlock *tsk, uint64_t va);
void* flat_mapping_block(struct taskControlBlock* tsk, uint64_t addr, uint64_t num_2m_blk);

void flush_tlb(uint64_t newpgd);

#endif

#endif

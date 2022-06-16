#ifndef MMU_H
#define MMU_H

#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1


#define PERIPHERAL_START 0x3c000000L
#define PERIPHERAL_END   0x3f000000L
#define PERIPHERAL_BASE 0x3f000000L
#define PAGE_TABLE_BASE 0x10000000L
#define USER_KERNEL_BASE 0x00000000L
#define THREAD_STACK_SIZE 0x4000L
#define USER_STACK_BASE 0xfffffffff000L - THREAD_STACK_SIZE

#define PGD_BASE (PAGE_TABLE_BASE + 0x0000)
#define PUD_BASE (PAGE_TABLE_BASE + 0x1000)
#define PMD_BASE (PAGE_TABLE_BASE + 0x2000)
#define PTE_BASE (PAGE_TABLE_BASE + 0x4000)

#define PD_TABLE 0b11L
#define PD_BLOCK 0b01L
#define PD_PAGE 0b11L
#define PD_NE_EL0 (1L << 54)
#define PD_NE_EL1 (1L << 53)
#define PD_ACCESS (1L << 10)
#define PD_UK_ACCESS (1L << 6)
#define PD_RDONLY    (1L << 7)
#define BOOT_PGD_ATTR (PD_TABLE)
#define BOOT_PUD_ATTR (PD_TABLE)
#define BOOT_PMD_ATTR (PD_TABLE)

#define BOOT_PTE_DEVICE_nGnRnE_ATTR \
  (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_PAGE | PD_UK_ACCESS | PD_NE_EL0 | PD_NE_EL1)
#define BOOT_PTE_NORMAL_NOCACHE_ATTR \
  (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_PAGE)

#define PHY_TO_VIR(x) ((x) | 0xffff000000000000L)
#define VIR_TO_PHY(x) ((x) & ~0xffff000000000000L)

#ifndef __ASSEMBLER__

#include "type.h"
#include "thread.h"

typedef struct thread thread_t;

typedef struct{
    unsigned int iss : 25, // Instruction specific syndrome
                 il : 1,   // Instruction length bit
                 ec : 6;   // Exception class
} esr_el1_t;

void init_page_table(uint64 **table, int level);
void set_page_tables_for_thread(thread_t *thread);
void free_page_tables_for_thread(thread_t *thread);
void map_page_table(thread_t *thread, uint64 *pgd, uint64 vir_addr, uint64 phy_addr, bool readOnly);
void parser_table(uint64 vir_addr);
void set_vm_list_for_thread(thread_t *thread);
void free_vm_list_for_thread(thread_t *thread);
void handle_abort(esr_el1_t *esr_el1);
void seg_fault();
#endif //__ASSEMBLER__

#endif
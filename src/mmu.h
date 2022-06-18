#ifndef __MMU_H__
#define __MMU_H__

#include "buddy.h"

#define PAGE_TABLE_BASE            (0x30000000)
#define PGD_BASE                   (PAGE_TABLE_BASE + 0x0000)
#define PUD_BASE                   (PAGE_TABLE_BASE + 0x1000)
#define PMD_BASE                   (PAGE_TABLE_BASE + 0x2000)
#define PTE_BASE                   (PAGE_TABLE_BASE + 0x4000)

#define TCR_CONFIG_REGION_48bit    (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB             ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT         (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE         (0b00000000)
#define MAIR_NORMAL_NOCACHE        (0b01000100)
#define MAIR_CONFIG                (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8))
#define MAIR_IDX_DEVICE_nGnRnE     (0)
#define MAIR_IDX_NORMAL_NOCACHE    (1)

#define PD_TABLE                   (0b11)
#define PD_BLOCK                   (0b01)
#define PD_PAGE                    (0b11)
#define PD_ACCESS                  (1 << 10)
#define PD_WRITE                   (1 << 7)
#define PD_USER                    (1 << 6)

#define BOOT_PGD_ATTR              PD_TABLE
#define BOOT_PUD_ATTR              PD_TABLE
#define BOOT_PMD_ATTR              PD_TABLE
#define BOOT_PTE_RAM_ATTR          (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_PAGE)
#define BOOT_PTE_DEVICE_ATTR       (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2)  | PD_PAGE)

#define PD_RAM_ATTR                (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK)
#define PD_USER_ATTR               (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK | PD_USER) 

#define PERIPHERAL_BASE            (0x3C000000)
#define PERIPHERAL_END             (0x3F000000)





#define PAGE_TABLE_SIZE            (4096)
#define PAGE_TABLE_ENTRY_NUM       (PAGE_TABLE_SIZE / 8)

#define MMU_DEBUG                  (0)


unsigned long va_to_pa (unsigned long va);
unsigned long pa_to_va (unsigned long pa);
unsigned long user_va_to_pa (unsigned long user_va);
unsigned long* create_page_table ();
void page_table_init (unsigned long *page_table);
void set_page_table (unsigned long *page_table, unsigned long offset, unsigned long next_level, unsigned long attribute);
unsigned long* alloc_page_table (unsigned long *pgd, unsigned long va, unsigned long pa, unsigned attribute);

#endif
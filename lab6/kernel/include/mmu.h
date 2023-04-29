#pragma once

#define KVA 0xffff000000000000
#define PERIPHERAL_BASE 0x3f000000

#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) | (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1

#define PAGE_TABLE_BASE 0x30000000
#define PGD_BASE (PAGE_TABLE_BASE + 0x0000)
#define PUD_BASE (PAGE_TABLE_BASE + 0x1000)
#define PMD_BASE (PAGE_TABLE_BASE + 0x2000)
#define PTE_BASE (PAGE_TABLE_BASE + 0x4000)

#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_PAGE 0b11
#define PD_ACCESS (1 << 10)
#define PD_USER_RW (0b01 << 6)
#define PD_USER_R (0b11 << 6)
#define PD_UXN (1L << 54)
#define PD_PXN (1L << 53)

#define BOOT_PGD_ATTR (PD_TABLE)
#define BOOT_PUD_ATTR (PD_TABLE)
#define BOOT_PMD_ATTR (PD_TABLE)
#define BOOT_PTE_DEVICE_nGnRnE_ATTR \
  (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_PAGE)
#define BOOT_PTE_NORMAL_NOCACHE_ATTR \
  (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_PAGE)

#include <stdint.h>
#include "thread.h"

#define VA2PA(addr) ((uint64_t)(addr) & (uint64_t)0x0000ffffffffffff)
#define PA2VA(addr) ((uint64_t)(addr) | (uint64_t)0xffff000000000000)

#define PERIPHERAL_START 0x3c000000
#define PERIPHERAL_END 0x3f000000

void init_page_table(thread_info *thread, uint64_t **table);
void update_page_table(thread_info *thread, uint64_t virtual_addr,
                       uint64_t physical_addr, uint64_t permission);
uint64_t el0_VA2PA(thread_info *thread, uint64_t virtual_addr);
                      
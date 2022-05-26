#include "stdint.h"

#define KVA 0xffff000000000000

#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1
#define MAIR_CONFIG_DEFAULT ((MAIR_DEVICE_nGnRnE<<(MAIR_IDX_DEVICE_nGnRnE*8))|(MAIR_NORMAL_NOCACHE<<(MAIR_IDX_NORMAL_NOCACHE*8)))

#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_ACCESS (1 << 10)
#define BOOT_PGD_ATTR PD_TABLE
#define BOOT_PUD_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)
#define BOOT_L2D_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)      // for L2 (peripherals)
#define BOOT_L2N_ATTR (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK)     // for L2 (normal)

#define VA2PA(x) ((uint64_t)(x) & 0xffffffffffff)
#define PA2VA(x) ((uint64_t)(x) | 0xffff000000000000)

void mmu_init();
void init_PT(uint64_t** page_table);
void map_pages(uint64_t* page_table, uint64_t va, uint64_t alloc);

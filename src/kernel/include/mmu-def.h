#ifndef _DEF_MMU_DEF
#define _DEF_MMU_DEF

#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1

#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_PAGE 0b11
#define PD_ACCESS (1 << 10)
#define PD_RO (1 << 7)
#define PD_USERACCESS (1 << 6)
#define PD_UNX (1 << 54)
#define PD_PNX (1 << 53)
#define BOOT_PGD_ATTR PD_TABLE
#define BOOT_PUD_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)
#define BOOT_PUD_TABLE PD_TABLE
#define BOOT_PMD_ATTR_NORMAL (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK)
#define BOOT_PMD_ATTR_DEVICE (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)
#define PROC_PTE_ATTR_NORMAL (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_PAGE)
#define PROC_PTE_ATTR_DEVICE (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_PAGE)

#define kernel_va_to_pa(x) ((x) & ~0xffff000000000000)
#define kernel_pa_to_va(x) ((x) | 0xffff000000000000)

//#define PROT_NONE 0
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4

#define MAP_ANONYMOUS    0x20
#define MAP_POPULATE     0x08000
#define MR_setflag(x, f) (x |= (f))
#define MR_unsetflag(x, f) (x &= (~f))

#endif
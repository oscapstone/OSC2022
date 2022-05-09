#ifndef __VM_H
#define __VM_H

//number of the most significant bits that must be either all 0s or all 1s
#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
//smallest block of memory that can be independently mapped in the translation tables
#define TCR_CONFIG_4KB ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE 0b00000000   //peripheral access
#define MAIR_NORMAL_NOCACHE 0b01000100  //normal RAM access
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1
#define MAIR_CONFIG_DEFAULT ((MAIR_DEVICE_nGnRnE<<(MAIR_IDX_DEVICE_nGnRnE*8))|(MAIR_NORMAL_NOCACHE<<(MAIR_IDX_NORMAL_NOCACHE*8)))

#define PD_TABLE 0b11
#define BOOT_PGD_ATTR PD_TABLE
#define PD_BLOCK 0b01
#define PD_ACCESS (1 << 10)  //access flag, a page fault is generated if not set
#define BOOT_PUD_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)  //for L1 (ARM peripherals)
#define BOOT_L2D_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)  //for L2 (peripherals)
#define BOOT_L2N_ATTR (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK) //for L2 (normal)


#endif
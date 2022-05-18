#include "mmu.h"

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

void mmu_init(){
  // Set up TCR_EL1
  asm volatile("msr tcr_el1, %[output]\n"::[output]"r"(TCR_CONFIG_DEFAULT));
  /*
		1000 0000 0001 0000 0000 0000 0001 0000
		[5:0] the number of the most significant bits that must be all 0s // TTBR0_EL0
		[21:16] the number of the most significant bits that must be all 1s // TTBR0_EL1
		[15:14] granule size for user space	(00=4KB, 01=16KB, 11=64KB) // TTBR0_EL0
		[31:30] granule size for kernel space // TTBR0_EL1
	*/
  // Set up mair_el1
  asm volatile("msr mair_el1, %[output]\n"::[output]"r"(MAIR_CONFIG_DEFAULT));
  /*
		0100 0100 0000 0000
		[7:0] device memory nGnRnE
		[15:8] normal memory without cache
	*/
  // L0 table init
	asm volatile("str %0, [%1]\n"::"r"(0x1000|BOOT_PGD_ATTR),"r"(0));  // 0 for the PGD's frame
  // L1 table init
  asm volatile("str %0, [%1]\n"::"r"(0x00000000|BOOT_PUD_ATTR),"r"(0x1000)); //finer granularity for different memory type
	asm volatile("str %0, [%1]\n"::"r"(0x40000000|BOOT_PUD_ATTR),"r"(0x1000+8)); // 1G block for ARM peripherals
  // Set Identity Paging
  // 0x00000000 ~ 0x3f000000: Normal
  // 0x3f000000 ~ 0x40000000: Device
  // 0x40000000 ~ 0x80000000: Device
  // load the PGD to the bottom translation-based register
  asm volatile("msr ttbr0_el1, %0\n"::"r"(0));
  // asm volatile("msr ttbr1_el1, %0\n"::"r"(0));
  // enable the mmu
  asm volatile("mrs x0, sctlr_el1\n");
  asm volatile("orr x0 , x0, 1\n");
  asm volatile("msr sctlr_el1, x0\n");
}

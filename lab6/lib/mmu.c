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
#define BOOT_L2D_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)      // for L2 (peripherals)
#define BOOT_L2N_ATTR (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK)     // for L2 (normal)

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
	asm volatile("str %0, [%1]\n"::"r"(0x2000|BOOT_PGD_ATTR),"r"(0x1000));             // 0x1000 for the PGD's frame
  // L1 table init
  asm volatile("str %0, [%1]\n"::"r"(0x3000|BOOT_PGD_ATTR),"r"(0x2000));        // finer granularity for different memory type
	asm volatile("str %0, [%1]\n"::"r"(0x40000000|BOOT_PUD_ATTR),"r"(0x2000+8));  // 1G block for ARM peripherals
  // Set Identity Paging
  // 0x00000000 ~ 0x3f000000: Normal
  // 0x3f000000 ~ 0x40000000: Device
  // 0x40000000 ~ 0x80000000: Device
  asm volatile("\
		mov x10, %0\n\
		mov x11, %1\n\
		mov x0, #0x3000\n\
		mov x1, #512\n\
		mov x2, #0\n\
  beg1:\n\
      cbz x1, end1\n\
      ldr x3, =0x3F000000\n\
      cmp x2, x3\n\
      blt normalmem\n\
  peripheralsmem:\n\
      orr x3, x2, x10\n\
      b end2\n\
  normalmem:\n\
      orr x3, x2, x11\n\
      b end2\n\
  end2:\n\
      str x3, [x0]\n\
      add x0, x0, #8\n\
      sub x1, x1, #1\n\
      add x2, x2, #0x200000\n\
      b beg1\n\
  end1:\n\
    "::"r"(BOOT_L2D_ATTR),"r"(BOOT_L2N_ATTR));
  // load the PGD to the bottom translation-based register
  asm volatile("msr ttbr0_el1, %0\n"::"r"(0x1000));
  asm volatile("msr ttbr1_el1, %0\n"::"r"(0x1000));
  // enable the mmu
  asm volatile("mrs x2, sctlr_el1\n");
  asm volatile("orr x2 , x2, 1\n");
  asm volatile("msr sctlr_el1, x2\n");
}

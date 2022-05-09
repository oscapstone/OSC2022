#include "vm.h"
#include "mini_uart.h"

void mmu_init() {
	//setup tcr & mair
	asm volatile("msr tcr_el1, %0\n"::"r"(TCR_CONFIG_DEFAULT));
	asm volatile("msr mair_el1, %0\n"::"r"(MAIR_CONFIG_DEFAULT));

	//L0 table init
	asm volatile("str %0, [%1]\n"::"r"(0x1000|BOOT_PGD_ATTR),"r"(0));

	//L1 table init
	asm volatile("str %0, [%1]\n"::"r"(0x2000|BOOT_PGD_ATTR),"r"(0x1000)); //finer granularity for different memory type
	asm volatile("str %0, [%1]\n"::"r"(0x40000000|BOOT_PUD_ATTR),"r"(0x1000+8)); // 1G block for ARM peripherals

	//L2 table for 0~1G (page frame size: 4MB)
	asm volatile("\
		mov x10, %0\n\
		mov x11, %1\n\
		mov x0, #0x2000\n\
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

	//setting L0 table for lower VA region (0000)
	asm volatile("msr ttbr0_el1, %0\n"::"r"(0));//ensure to read correct inst when mmu opened

	//setting L0 table for higher VA region (ffff)
	asm volatile("msr ttbr1_el1, %0\n"::"r"(0));

	//enalble mmu
	asm volatile("\
		mrs x2, sctlr_el1\n\
		orr x2 , x2, 1\n\
		msr sctlr_el1, x2\n\
	"::);

	//no longer running on 0x80000
	asm volatile("\
		ldr x0, =0xffff000000000000\n\
		add x30, x30, x0\n\
	"::);
}
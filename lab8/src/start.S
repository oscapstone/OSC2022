        .section ".text.boot"
        .globl _start   
        .extern dtb_addr

#include "mmu.h"
        
_start:
        // move devtree addr to x11
        mov x11, x0
        // read cpu id
        mrs x0, mpidr_el1
        // only use core0, the other core goes into infinite loop
        and x0, x0, #0xFF  
        cbz x0, _core0
_loop:
        wfe
        b _loop
        
_core0:
        // set stack top to the text section's head
        bl _from_el2_to_el1
        ldr x0, =_start
        // mov x0, #0x0000
        // movk x0, #7, lsl #16
        mov sp, x0

        // initialize bss section
        // the size of the bss section should align to 8,
        // or the following routine become an infinite loop.

        adr x0, __bss_begin
        adr x1, __bss_end
        sub x1, x1, x0
        cbz x1, _kernel
_zero:
        str xzr, [x0], #8
        sub x1, x1, #8
        cbnz x1, _zero


_setup_mmu:
        ldr x0, = TCR_CONFIG_DEFAULT
        msr tcr_el1, x0

        ldr x0, =( \
          (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
          (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) \
        )
        msr mair_el1, x0

        mov x0, #0x0000 // PGD's page frame at 0x40000
        movk x0, #4, lsl #16
        mov x1, #0x1000 // PUD's page frame at 0x41000
        movk x1, #4, lsl #16

        ldr x2, =BOOT_PGD_ATTR
        orr x2, x1, x2 // combine the physical address of next level page with attribute.
        str x2, [x0]
        
        ldr x2, =BOOT_DEV_ATTR
        mov x3, #0x40000000
        orr x3, x2, x3
        str x3, [x1, 8] // 2nd 1GB mapped by the 2nd entry of PUD
        
        mov x2, #0x2000
        movk x2, #4, lsl #16 // PMD's page frame at 0x42000
        orr x3, x2, #PD_TABLE
        str x3, [x1]
        // mov x3, #0x00000000
        // orr x3, x2, x3
        // str x3, [x1] // 1st 1GB mapped by the 1st entry of PUD

        // flat map 0x0 ~ 0x3f00_0000 as normal ram 0x3f00_0000 ~ 0x4000_0000 as device memory
        // require x2 as PMD address
        // x3 as attribute, x1 as target addr, x4 as counter, count from [0, 512), x5 tmp
        // 0~504
        ldr x3, =BOOT_NORM_ATTR
        mov x1, #0x0
        mov x4, #504

__normal_ram_loop:      
        orr x5, x1, x3
        str x5, [x2]
        
        add x2, x2, #8
        add x1, x1, #512, lsl #12
        sub x4, x4, #1
        cbnz x4, __normal_ram_loop
        

        // 504~511
        ldr x3, =BOOT_DEV_ATTR
        mov x1, #0x3F000000
        mov x4, #8
        
__device_mem_loop:      
        orr x5, x1, x3
        str x5, [x2]
        
        add x2, x2, #8
        add x1, x1, #512, lsl #12
        sub x4, x4, #1
        cbnz x4, __device_mem_loop

        msr ttbr0_el1, x0 // load PGD to the bottom translation-based register.
        msr ttbr1_el1, x0 // also load PGD to the upper translation based register.

        mrs x2, sctlr_el1
        orr x2 , x2, 1
        msr sctlr_el1, x2 // enable MMU, cache remains disabled
        
        ldr x2, =_kernel
        br x2
_kernel:
        // store devtree addr to the global extern variable
        // in kernel virtual address
        ldr x1, =dtb_addr
        mov x0, #VA_START
        add x11, x11, x0
        str x11, [x1]
        bl kernel_main
        b  _loop
        
_from_el2_to_el1:
        mov x0, (1 << 31)
        msr hcr_el2, x0
        // The Execution state for EL1 is AArch64.
        // The Execution state for EL0 is determined by the current value of PSTATE.nRW when executing at EL0.
        // mov x0, 0x3c5
        mov x0, 0x345
        // 3c5 = 0011 1100 0101
        // 345 = 0011 0100 0101
        // 0011 : branch type indicator. debug mask, SError interrupt mask.
        // 1100 : irq interrupt mask, fiq interrupt mask, reserved. exec state (0 = aarch64)
        // 0101 : sp_el1h
        msr spsr_el2, x0
        msr elr_el2, lr
        // set the eret return adress
        eret

#include "mmu.h"
.section ".text.boot"

.global _start

// x0 register stores address to dtb

_start:
    mrs     x1, mpidr_el1   // get system register
    and     x1, x1, #3      // mask out bits not belonging to core id
    cbz     x1, 2f          // branch if result is zero (core id matched)

1:  wfe                     // cpu id > 0, stop
    b       1b

2:
    bl      from_el2_to_el1

    // set tcr_el1 to TCR_CONFIG_DEFAULT (0x0000000080100010)
    movz    x4, 0x0010
    movk    x4, 0x8010, lsl 16
    msr tcr_el1, x4

    // set memory attributes to MAIR_CONFIG_DEFAULT (0x0000000000004400)
    movz    x4, 0x4400
    msr mair_el1, x4

    // enable MMU
    mov x4, 0x1000 // PGD's page frame at 0x1000
    mov x1, 0x2000 // PUD's page frame at 0x2000

    // ldr x2, = BOOT_PGD_ATTR
    movz    x2, 0x0003
    orr x2, x1, x2 // combine the physical address of next level page with attribute.
    str x2, [x4]

    // ldr x2, = BOOT_PUD_ATTR
    movz    x2, 0x0401
    mov x3, 0x00000000
    orr x3, x2, x3
    str x3, [x1]    // 1st 1GB mapped by the 1st entry of PUD
    mov x3, 0x40000000
    orr x3, x2, x3
    str x3, [x1, 8] // 2nd 1GB mapped by the 2nd entry of PUD

    msr ttbr0_el1, x4 // load PGD to the bottom translation-based register.
    msr ttbr1_el1, x4 // also load PGD to the upper translation based register.

    mov sp, 0x3c000000
    bl set_2M_kernel_mmu

    mrs x2, sctlr_el1
    orr x2 , x2, 1
    msr sctlr_el1, x2 // enable MMU, cache remains disabled

    // indirect branch to the upper VA
    ldr x2, =set_exception_vector_table
    br  x2

set_exception_vector_table:
    adr     x1, exception_vector_table
    msr     vbar_el1, x1

    // set top of stack at 0xffff00003c000000 (last usable memory)
    movz    x3, 0x0000
    movk    x3, 0x3c00, lsl 16
    movk    x3, 0xffff, lsl 48
    mov     sp, x3

    // clear bss
    ldr     x1, =_bss_start
    ldr     w2, =_bss_size
3:
    cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

4:
    // jump to C code
    bl      kernel_main
    // else, halt this core
    b       1b

from_el2_to_el1:
    mov     x1, (3 << 20)   // FPEN : access the Advanced SIMD and floating-point registers
    msr     CPACR_EL1, x1
    mov     x1, (1 << 31)   // EL1 uses aarch64
    msr     hcr_el2, x1
    mov     x1, 0x3c5       // EL1h (SPSel = 1) with interrupt disabled
    msr     spsr_el2, x1
    msr     elr_el2, lr
    eret                    // return to EL1

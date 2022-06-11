#include "mmu.h"

.section ".text"
.global _start // decdie entry point

_start: 
    // only run program on core-0, others(1-3) are idle 
    mrs x1, mpidr_el1 // core identification, [7:0] - individual threads within a core
    and x1, x1, #3  // 3 => 0~3 (rpi3 has 4 cores)
    cbz x1, core0 // run progrm if core=0 else idle

idle:  
    wfe // wait for event (idle)
    b idle

core0: 
    bl from_el2_to_el1 // switch exception level from 2 to 1 (Must at begining)

    // set paging configuration (up : 0xffff000000000000 low : 0x0000000000000000)
    ldr x0, = TCR_CONFIG_DEFAULT
    msr tcr_el1, x0

    // set used memory attributes
    ldr x0, =( \
    (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
    (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) \
    )
    msr mair_el1, x0

    mov sp, 0x3c000000
    bl set_PGD
    bl set_PUD
    bl set_PMD
    bl set_PTE

    ldr x0, =PGD_BASE
    msr ttbr0_el1, x0 // load PGD to the bottom translation-based register.
    msr ttbr1_el1, x0 // also load PGD to the upper translation based register.

    mrs x2, sctlr_el1
    orr x2 , x2, 1
    msr sctlr_el1, x2 // enable MMU, cache remains disabled

    ldr x2, =set_exception_vector_table // indirect branch to the virtual address
    br x2

set_exception_vector_table:
    adr x0, exception_vector_table
    msr vbar_el1, x0

    // set the stack pointer at the top of memory (virtual address: 0xffff00003c000000)
    movz x0, 0x0000
    movk x0, 0x3c00, lsl 16 // 3c000000 is the size of memory
    movk x0, 0xffff, lsl 48 // 0xffff000000000000 is the start address of virtual kernel address
    mov sp, x0

    // set the uninitialized variable (bss section) to zero
    // x - 64bits, w - 32bits (4bytes is large enough for size)
    ldr x1, =_bss_start // _bss_start described in linker script
    ldr w2, =_bss_size // _bss_size described in linker script


init_loop: 
    cbz w2, main_loop
    str xzr, [x1], #8 // set x1 to zero(xzr), and x1's address + 8bytes 
    sub w2, w2, #1
    b init_loop

main_loop:
    bl main // branch with link (main function in c code, a while loop)
    b idle // just in case fail

from_el2_to_el1:
    mov x0, #(3 << 20) // Make el0, el1 can use Floating point and Advanced SIMD
    msr cpacr_el1, x0
    mov x0, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x0
    mov x0, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x0
    msr elr_el2, lr
    eret // return to EL1

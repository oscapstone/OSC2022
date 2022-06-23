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

set_exception_vector_table:
    adr     x1, exception_vector_table
    msr     vbar_el1, x1

    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
    // ldr     x1, =_start
    // mov     sp, x1
    // set top of stack at 0x3c000000 (last usable memory)
    mov     sp, 0x3c000000

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

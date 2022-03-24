.section ".text.boot"
.global _start

_start:
    mov     x10, x0
    mrs     x1, mpidr_el1 // get system register
    and     x1, x1, #3 // mask out bits not belonging to core id
    cbz     x1, 2f // branch if result is zero (core id matched)

// cpu id > 0, stop
1:  wfe
    b       1b

// cpu id == 0
// Set stack to start below the .text section
2:  adr     x2, _exception_vector_table
    msr     vbar_el1, x2
    bl      _from_el2_to_el1

    ldr     x1, =_start
    mov     sp, x1
    
// Get bss info
3:  ldr     x1, =__bss_start
    ldr     w2, =__bss_size

// Set bss to zero
4:  cbz     w2, 5f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 4b

// Run main()
5:  mov     x0, x10
    bl      main
    b       1b 


exception_handler:
    eret


.align 11 // vector table should be aligned to 0x800
_exception_vector_table:
    b exception_handler // branch to a handler function.
    .align 7 // entry size is 0x80, .align will pad 0
    b exception_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7

    b exception_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7

    b exception_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7

    b exception_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7


_from_el2_to_el1:
    mov x0, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x0
    mov x0, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x0
    msr elr_el2, lr
    eret // return to EL1


_from_el1_to_el0:
    mov x0, 0x3c0 // EL0t
    msr spsr_el1, x0
    msr elr_el1, lr
    eret // return to EL0
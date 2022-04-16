.section ".text.boot"

.global _start

_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // cpu id > 0, stop
1:  wfe
    b       1b
2:  // cpu id == 0

    // set top of stack at ARM memory end: 0x3C000000
    ldr     x1, =0x3C000000
    mov     sp, x1

    // clear bss
    ldr     x1, =__bss_start
    ldr     x2, =__bss_end
    sub     x2, x2, x1
3:  cbz     x2, 4f
    str     xzr, [x1], #8
    sub     x2, x2, #8
    cbnz    x2, 3b

4:  // loading fdt-address at register x0
    ldr x1, =_fdt_ptr
    str x0, [x1]

    // jump to C code, should not return
    bl      main
    // for failsafe, halt this core too
    b       1b

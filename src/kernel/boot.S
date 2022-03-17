.section ".text.boot"

.global _start

_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // if cpu id > 0, stop
1:  wfe
    b       1b
2:  // else if cpu id == 0
    // set the stack pointer just before out code
    ldr     x1, =_start
    mov     sp, x1

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f
    str     xzr, [x1], #8    // set content in [x1] to zero
    sub     w2, w2, #1       // decrease bss size 8 bytes
    cbnz    w2, 3b
    // jump to C code
4:  bl      main
    // for failsafe, halt this core too
    b       1b
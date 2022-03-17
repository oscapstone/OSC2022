.section ".text.boot"

.global _start

_start:
    mov     x10, x0
    mov     x11, x1
    mov     x12, x2
    mov     x13, x3

    ldr     x1, =0x80000
    ldr     x2, =_start
    ldr     x3, =_end
    sub     x3, x3, x2
1:  ldr     x4, [x1], #8
    str     x4, [x2], #8
    sub     x3, x3, #8
    cbnz    x3, 1b

    // set top of stack just before our code
    ldr     x1, =_start
    mov     sp, x1

    // clear bss
    ldr     x1, =__bss_start
    ldr     x2, =__bss_end
    sub     x2, x2, x1
3:  cbz     x2, 4f
    str     xzr, [x1], #8
    sub     x2, x2, #8
    cbnz    x2, 3b

4:  // jump to relocated C code, should not return
    bl      main - 0x20000

5:  wfe
    b       5b
    
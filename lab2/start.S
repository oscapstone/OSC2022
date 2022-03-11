
.section ".text"

.global _start

_start:
    b       2f
1:  wfe
    b       1b
2:  // cpu id == 0

    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
    ldr     x1, =_start
    mov     sp, x1

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f
    str     xzr, [x1], #8 // put xzr to x1 then x1 = x1 + 8  xzr zero register
    sub     w2, w2, #1
    cbnz    w2, 3b
    
4:  bl      main
    b       1b

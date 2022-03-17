.section ".text"

.global _start

_start:

    ldr     x1, =__start
    mov     sp, x1

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
clear_bss_loop:  
    cbz     w2, main_code
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, clear_bss_loop

    // jump to C code, should not return
main_code:  bl      main
    // for failsafe, halt this core too
    b       _start
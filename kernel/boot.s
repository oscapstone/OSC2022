.section ".text.boot"

.global _start

// x0 register stores address to dtb

// clear bss
_start:
    ldr     x20, =_bss_start
    ldr     x21, =_bss_size
    add     x21, x21, x20
3:  
    cmp     x20, x21
    beq     4f
    str     xzr, [x20]
    add     x20, x20, #8
    b       3b

// set sp just before our code
// jump to C code
4:
    ldr     x20, =_start
    mov     sp, x20
    bl      kernel_main

// else, halt this core
5:
    b       5b

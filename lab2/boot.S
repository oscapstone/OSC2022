.section ".text.boot"

.global _start

_start:
    adr     x20, .
    ldr     x21, =_bl_size
    add     x21, x21, x20
    ldr     x22, =_text_start

    // Relocate the bootloader itself from 0x80000 to 0x60000.
    // This way, 0x80000 can be used for kernel.img.
1:
    cmp     x20, x21
    b.eq    2f
    ldr     x23, [x20]
    str     x23, [x22]
    add     x20, x20, #8
    add     x22, x22, #8
    b       1b

2:
    // Jump to the ".text._bl" section after relocation
    ldr     x1, =_bl_start
    br      x1 

.section ".text._bl"
    // clear bss
    ldr     x20, =_bss_start
    ldr     x21, =_bss_size
    add     x21, x21, x20
3:  
    cmp     x20, x21
    b.eq    4f
    str     xzr, [x20]
    add     x20, x20, #8
    b       3b

    // jump to C code, should not return
    bl      kernel_main
4:  
    // for failsafe, halt this core too
    b       4b
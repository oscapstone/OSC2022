.section ".text.boot"

.global _start

// x0 register stores dtb address

// x20 = 0x80000, x21 = end of bootloader
// x19 = x22 = 0x60000
_start:
    adr     x20, .
    ldr     x21, =_bl_size
    add     x21, x21, x20
    ldr     x22, =_text_start
    mov     x19, x22

// relocate the bootloader from 0x80000 to 0x60000
1:
    cmp     x20, x21
    beq     2f
    ldr     x23, [x20]
    str     x23, [x22]
    add     x20, x20, #8
    add     x22, x22, #8
    b       1b

// jump to the ".text._bl" section after relocation
2:
    ldr     x1, =_bl_start
    br      x1 

.section ".text._bl"
// set sp just before our code (0x60000)
    mov     sp, x19

// clear bss
    ldr     x20, =_bss_start
    ldr     x21, =_bss_size
    add     x21, x21, x20
3:  
    cmp     x20, x21
    beq     4f
    str     xzr, [x20]
    add     x20, x20, #8
    b       3b

// jump to C code
4:   
    bl      bootloader_main

// else, halt this core
5:
    b       5b

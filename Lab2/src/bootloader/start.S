.section ".text.boot"

.global _start

__relocate_addr = 0x60000

_start:
    mrs     x1, mpidr_el1  // processor ID
    and     x1, x1, #3    
    cbz     x1, 2f     // only use the processor with ID 0
    
1:  wfe
    b       1b    // processor ID > 0, loop....

    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
2:  ldr     x1, = __relocate_addr    
    mov     sp, x1

    ldr     x1, =__relocate_addr
    ldr     x2, =__bootloader_size
    ldr     x3, =__bootloader_start

// copy the bootloader text at 0x80000 to 0x60000
relocate:
    ldr     x4, [x3], #8
    str     x4, [x1], #8
    sub     x2, x2, #1
    cbnz    x2, relocate    

    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f      // bss size = 0, go to main function
    str     xzr, [x1], #8    // set x1 to the zero register, for every 8 byte
    sub     w2, w2, #1       
    cbnz    w2, 3b      //if bss size > 0, go to loop

4:  bl      boot_main-0x20000
    b       1b

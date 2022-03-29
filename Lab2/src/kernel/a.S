.section ".text.boot"

.global _start

_start:
    mrs     x1, mpidr_el1  // processor ID
    and     x1, x1, #3    
    cbz     x1, 2f     // only use the processor with ID 0
    
1:  wfe
    b       1b    // processor ID > 0, loop....
    
2:  ldr     x1, = _start    
    mov     sp, x1

    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f      // bss size = 0, go to main function
    str     xzr, [x1], #8    // set x1 to the zero register, for every 8 byte
    sub     w2, w2, #1       
    cbnz    w2, 3b      //if bss size > 0, go to loop

4:  bl      main
    b       1b

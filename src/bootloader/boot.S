.section ".text.boot"

.global _start

__relocate_address = 0x60000

_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3

    // if cpu id > 0, stop
    cbnz    x1, stop

    // set the stack pointer just before out code
    ldr     x1, =_start
    mov     sp, x1

    // relocate our code from load address to link address
    ldr     x1, =0x80000
    ldr     x2, =__relocate_address
    ldr     w3, =__loader_size
    // if loader size == 0, go to 
    cbz     w3, uart_boot

relocate_loop:  
    ldr     x4, [x1], #8
    str     x4, [x2], #8
    sub     w3, w3, #1
    cbnz    w3, relocate_loop

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
clear_bss:  
    cbz     w2, boot
    str     xzr, [x1], #8    // set content in [x1] to zero
    sub     w2, w2, #1       // decrease bss size 8 bytes
    cbnz    w2, clear_bss

boot:  
    // jump to C code
    bl      uart_boot-0x20000
    // for failsafe, halt this core too
    b       stop

stop:  
    wfe
    b       stop
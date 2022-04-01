.section ".text"
.global _start // decdie entry point

_start: 
    bl from_el2_to_el1 // switch exception level from 2 to 1 (Must at begining)
    // only run program on core-0, others(1-3) are idle 
    mrs x1, mpidr_el1 // core identification, [7:0] - individual threads within a core
    and x1, x1, #3  // 3 => 0~3 (rpi3 has 4 cores)
    cbz x1, core0 // run progrm if core=0 else idle

idle:  
    wfe // wait for event (idle)
    b idle

core0: 
    // set the stack pointer at the address of code
    ldr x1, =_start // get address of code
    mov sp, x1 // set stack pointer 

    // set the uninitialized variable (bss section) to zero
    // x - 64bits, w - 32bits (4bytes is large enough for size)
    ldr x1, =_bss_start // _bss_start described in linker script
    ldr w2, =_bss_size // _bss_size described in linker script

set_exception_vector_table:
    adr x0, exception_vector_table
    msr vbar_el1, x0

init_loop: 
    cbz w2, main_loop
    str xzr, [x1], #8 // set x1 to zero(xzr), and x1's address + 8bytes 
    sub w2, w2, #1
    b init_loop

main_loop:
    bl main // branch with link (main function in c code, a while loop)
    b idle // just in case fail

from_el2_to_el1:
    mov x0, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x0
    mov x0, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x0
    msr elr_el2, lr
    eret // return to EL1

.section ".text"
.global _start // decdie entry point

_start: 
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

init_loop: 
    cbz w2, main_loop
    str xzr, [x1], #8 // set x1 to zero(xzr), and x1's address + 8bytes 
    sub w2, w2, #1
    b init_loop

main_loop:
    bl main // branch with link (main function in c code, a while loop)
    b idle // just in case fail


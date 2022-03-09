.section ".text.boot"
.globl _start
        
_start:
        // read cpu id
        mrs x0, mpidr_el1
        // only use core0, the other core goes into infinite loop
        and x0, x0, #0xFF
        cbz x0, _core0
_loop:
        wfe
        b _loop
_core0:
        // set stack top to the text section's head
        ldr x0, =_start
        mov sp, x0

        // initialize bss section
        adr x0, __bss_begin
        adr x1, __bss_end
        sub x1, x1, x0
_zero:
        cbz w1, _kernel
        str xzr, [x0], #8
        sub x1, x1, #8
        cbnz x1, _zero
_kernel:
        bl kernel_main
        b  _loop

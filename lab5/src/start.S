        .section ".text.boot"
        .globl _start   
        .extern dtb_addr
        
_start:
        // move initrd addr to x11
        mov x11, x0
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
        bl _from_el2_to_el1
        ldr x0, =_start
        // mov x0, #0x0000
        // movk x0, #7, lsl #16
        mov sp, x0

        // initialize bss section
        adr x0, __bss_begin
        adr x1, __bss_end
        sub x1, x1, x0
_zero:
        cbz x1, _kernel
        str xzr, [x0], #8
        sub x1, x1, #8
        cbnz x1, _zero
_kernel:
        // store initrd addr to the global extern variable
        ldr x1, =dtb_addr
        str x11, [x1]
        bl kernel_main
        b  _loop
_from_el2_to_el1:
        mov x0, (1 << 31)
        msr hcr_el2, x0
        // The Execution state for EL1 is AArch64.
        // The Execution state for EL0 is determined by the current value of PSTATE.nRW when executing at EL0.
        // mov x0, 0x3c5
        mov x0, 0x345
        // 3c5 = 0011 1100 0101
        // 345 = 0011 0100 0101
        // 0011 : branch type indicator. debug mask, SError interrupt mask.
        // 1100 : irq interrupt mask, fiq interrupt mask, reserved. exec state (0 = aarch64)
        // 0101 : sp_el1h
        msr spsr_el2, x0
        msr elr_el2, lr
        // set the eret return adress
        eret

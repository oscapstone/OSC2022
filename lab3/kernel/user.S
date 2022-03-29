.globl branch_to_address_in_el0
branch_to_address_in_el0:
        // save return address and other status
        // save_all (deleted)
        // x0 begin_addr, x1 stack_addr
        mov x2, 0x340
        // 0x3c0 = 0011 1100 0000
        // 0x340 = 0011 0100 0000 (enable IRQ)
        // branch type indicator [11:10]
        // debug exception, SError, IRQ, FIQ interrupt mask [9:6]
        // aarch64 [4]
        // Spsel 0b0000 = EL0t [3:0]
        msr spsr_el1, x2
        msr elr_el1, x0
        msr sp_el0, x1
        eret

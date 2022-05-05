#define CORE0_TIMER_IRQ_CTRL 0x40000040

.globl core_timer_enable
core_timer_enable:
        mov x0, 1
        msr cntp_ctl_el0, x0 // enable
        mov x0, #0xffff
        movk x0, #0x7fff, lsl #16
        msr cntp_cval_el0, x0 // set expired time
        mov x0, 2
        ldr x1, =CORE0_TIMER_IRQ_CTRL
        str w0, [x1] // unmask timer interrupt
        ret

.globl reset_timer
reset_timer:
        msr cntp_tval_el0, x0
        ret

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

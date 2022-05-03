.equ CORE0_TIMER_IRQ_CTRL, 0x40000040

.globl timer_init
timer_init:
    mov x0, 1
    msr cntp_ctl_el0, x0
    mrs x0, cntfrq_el0
    lsr x0, x0, #5
    msr cntp_tval_el0, x0 
    mov x0, 2
    ldr x1, =CORE0_TIMER_IRQ_CTRL
    str w0, [x1]
    ret
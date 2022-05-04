.global context_switch_to
.extern reboot_now
.extern uart_putshex
context_switch_to:
    stp x19, x20, [x0, 16 * 0]
    stp x21, x22, [x0, 16 * 1]
    stp x23, x24, [x0, 16 * 2]
    stp x25, x26, [x0, 16 * 3]
    stp x27, x28, [x0, 16 * 4]
    stp fp, lr, [x0, 16 * 5]
    mov x9, sp
    //mrs x10, sp_el0
    //mrs x11, elr_el1
    stp x9, x10, [x0, 16 * 6]
    str x11, [x0, 16 * 7]

    ldp x19, x20, [x1, 16 * 0]
    ldp x21, x22, [x1, 16 * 1]
    ldp x23, x24, [x1, 16 * 2]
    ldp x25, x26, [x1, 16 * 3]
    ldp x27, x28, [x1, 16 * 4]
    ldp fp, lr, [x1, 16 * 5]
    ldp x9, x10, [x1, 16 * 6]
    ldr x11, [x1, 16 * 7]
    //mov x0, x9
    //bl uart_putshex
    mov sp,  x9
    //msr sp_el0, x10
    //msr elr_el1, x11
    msr tpidr_el1, x2
    //bl reboot_now
    ret

.global thread_get_current
thread_get_current:
    mrs x0, tpidr_el1
    ret
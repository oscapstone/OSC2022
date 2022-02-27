.section .text
.global _start
_start:
    ldr x0, =0
    ldr x1, =0
    ldr x2, =0
    ldr x3, =0
    ldr x4, =0
    ldr x5, =0
    ldr x6, =0
    ldr x7, =0
    ldr x8, =0
    ldr x9, =0
    ldr x10, =0
    ldr x11, =0
    ldr x12, =0
    ldr x13, =0
    ldr x14, =0
    ldr x15, =0
    ldr x16, =0
    ldr x17, =0
    ldr x18, =0
    ldr x19, =0
    ldr x20, =0
    ldr x21, =0
    ldr x22, =0
    ldr x23, =0
    ldr x24, =0
    ldr x25, =0
    ldr x26, =0
    ldr x27, =0
    ldr x28, =0
    ldr x29, =0
    ldr x30, =0

    // Read cpu id, stop slave cores
    mrs x1, mpidr_el1
    and x1, x1, #3
    cbz x1, 2f
1:
    wfe
    b   1b
2:

init_bss:
    ldr x0, =__bss_start
    ldr x1, =__bss_end
    ldr x2, =0x0
    bl  fill_block

write_stack_pattern:
    ldr x0, =_stack_end
    ldr x1, =__stack - 4
    ldr x2, =0x0
    bl fill_block

init_stack:
    ldr x0, =_stack
    mov sp, x0

SystemInit:
    bl main

SystemExit:

dead_loop:
    b dead_loop

fill_block:
    strh w2, [x0]
    cmp x0, x1
    b.eq fb_end
    add x0, x0, #4
    b fill_block

fb_end:
    ret










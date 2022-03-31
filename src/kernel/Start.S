.section ".text.start"
.extern _stack_end
.extern _init
.extern _DTB_ADDRESS
.global _start
_start:
    ADR X1, _DTB_ADDRESS
    STR X0, [X1]
    ADRP X0, _stack_end
    MOV SP, X0
    MOV X29, X0
    bl from_el2_to_el1
    ADRP X0, _stack_end
    MOV SP, X0
    MOV X29, X0
    bl _init

from_el2_to_el1:
    mov x0, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x0
    mov x0, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x0
    msr elr_el2, lr
    eret // return to EL1
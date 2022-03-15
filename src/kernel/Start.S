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
    bl _init
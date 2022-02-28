.section ".text.start"
.extern _stack_end
.extern _init
.global _start
_start:
    ADRP X0, _stack_end
    MOV SP, X0
    MOV X29, X0
    bl _init
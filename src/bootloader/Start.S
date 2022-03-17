.section ".text.start"
.extern _stack_end
.extern _init
.extern _bootloader_start
.extern _code_start
.extern _kernel_start
.global _start
_start:
    ADRP X0, _stack_end
    ADRP X1, _bootloader_start
    ADRP X2, _code_start
    SUB X3, X2, X1
    MOV X25, X3
    ADD X0, X0, X3
    MOV SP, X0
    MOV X29, X0
    bl _init
    MOV X0, X24
    ADRP X1, _kernel_start
    ADD X1, X1, X25
    br X1

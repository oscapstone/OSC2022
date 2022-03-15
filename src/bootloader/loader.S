.section ".text.loader"
.extern _stack_end
.extern _bootloader_start
.extern _code_start
.extern _code_end
.extern _data_start
.extern _data_end
.global _loader
_loader:
    MOV X24, X0
    ADRP X0, _stack_end
    MOV SP, X0
    MOV X29, X0
    ADRP X0, _code_start
    ADRP X3, _code_end
    ADRP X1, _bootloader_start
    .loop_code:
        LDR X2, [X0], #8
        STR X2, [X1], #8
        CMP X0, X3
        B.LO .loop_code
    ADRP X0, _data_start
    ADRP X3, _data_end
    ADRP X1, _bootloader_start
    ADRP X4, _code_start
    SUB X5, X0, X4
    ADD X1, X1, X5
    .loop_data:
        LDR X2, [X0], #8
        STR X2, [X1], #8
        CMP X0, X3
        B.LO .loop_data
    B _bootloader_start
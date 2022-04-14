_load_start = 0x80000; /* default entry address */
_heap_size = 0x10000;
_user_stack_size = 0x20000;

SECTIONS
{
    . = _load_start;
    .text : { KEEP(*(.text.boot)) *(.text .text.* .gnu.linkonce.t*) }
    .rodata : { *(.rodata .rodata.* .gnu.linkonce.r*) }
    PROVIDE(_data = .);
    .data : { *(.data .data.* .gnu.linkonce.d*) }
    .bss : {
        . = ALIGN(16);
        _bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        /* align is to ensure the size of bss section
         is times of 64bits. Use 16bytes is a common strategy */
        . = ALIGN(16); 
        _bss_end = .;
    }
    _end = .;
    _heap_start = .;
    _heap_end = _heap_start + _heap_size;
    _user_program = _heap_end + _user_stack_size;

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}

/*>>3 = /8 => 8bytes*/ 
_bss_size = (_bss_end - _bss_start)>>3;
_loader_size = (_end - _load_start)>>3; 

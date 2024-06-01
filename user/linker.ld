SECTIONS
{
    . = 0x80000;
    .text :  { *(.text) }
    
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    . = ALIGN(8);
    bss_begin = .;
    .bss : { *(.bss*) } 
    bss_end = .;
    _end = .;
    
    _head_start_brk = .;

}

_boot_size = _end - _start;
_head_start_brk = _head_start_brk + 0x00100;
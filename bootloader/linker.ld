SECTIONS
{
    . = 0x60000;
    _relocate_addr = .;
    . = 0x80000;
    .text :  { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    . = ALIGN(8);
    bss_begin = .;
    .bss : { *(.bss*) } 
    bss_end = .;
    _end = .;

}

_boot_size = _end - _start;
SECTIONS
{
    . = 0xffff000000000000; /* kernel space */
    . += 0x80000; /* kernel load address */
    _load_start = . ;
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

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}

/*>>3 = /8 => 8bytes*/ 
_bss_size = (_bss_end - _bss_start)>>3;
_loader_size = (_end - _load_start)>>3; 

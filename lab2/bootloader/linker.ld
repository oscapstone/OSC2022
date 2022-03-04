SECTIONS
{
    /*bootloader on 0x80000, kernel on 0x80000 need to self relocate bootloader to other place*/
    . = 0x80000;
    PROVIDE(_start = .);
    .text : { KEEP(*(.text.boot)) *(.text .text.* .gnu.linkonce.t*) }
    .rodata : { *(.rodata .rodata.* .gnu.linkonce.r*) }
    PROVIDE(_data = .);
    .data : { *(.data .data.* .gnu.linkonce.d*) }
    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
    _end = .;

   . = 0x90000;
   _code_relocate_place = .;
   /* _kernel_start_addr = 0x90000; */
   /* dont need this, because of using self relocation*/
}
__bss_size = (__bss_end - __bss_start)>>3;
__code_size = (_end - _start);

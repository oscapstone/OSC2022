SECTIONS
{
  . = 0x80000;
  .text : { KEEP(*(.text.boot)) *(.text .text.*) }
  .rodata : { *(.rodata .rodata.*) }
  .data : { *(.data) }
  .bss : {
    . = ALIGN(0x8);
    __bss_begin = .; 
    *(.bss*)
    __bss_end = .; 
  }
}

__bss_size = (__bss_end - __bss_begin)>>3;

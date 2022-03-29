SECTIONS
{
  . = 0x150000;
  .text : { KEEP(*(.text.boot)) *(.text .text.*) }
  .rodata : { *(.rodata .rodata.*) }
  .data : { *(.data) }
  . = ALIGN(8);
  __bss_begin = .;
  .bss : { *(.bss*) }
  . = ALIGN(8);
  __bss_end = .;
}

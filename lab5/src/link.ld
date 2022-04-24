SECTIONS
{
  . = 0x80000;
  __kernel_begin = .;
  .text : { KEEP(*(.text.boot)) *(.text .text.*) }
  .rodata : { *(.rodata .rodata.*) }
  .data : { *(.data) }
  . = ALIGN(8);
  __bss_begin = .;
  .bss : { *(.bss*) }
  . = ALIGN(8);
  __bss_end = .;
  heap_begin = .;
}

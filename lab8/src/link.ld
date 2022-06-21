SECTIONS
{
  . = 0xffff000000000000;
  . += 0x80000;
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

/*
0x0 ~ 0x3e00_0000 : Normal Memory
0x3e00_0000 ~ 0x3fff_ffff: GPU peripheral - 32M
0x4000_0000 ~ 0x7fff_ffff: Local peripherals - 1G

need 1 pgd
need 1 pud (2 Entries, one point to pmd, one point to 1G Block)
need 1 pmd (point to 512 2MB block)
*/

_STACK_SIZE = 0x2000;
_HEAP_SIZE  = 0x1000;
_PHY_SIZE = 0x32000;
MEMORY
{
    rom (rwx) : ORIGIN = 0x80000, LENGTH = 128K
    ram (rwx) : ORIGIN = 0xa0000, LENGTH = 32K
}

ENTRY(_start)

SECTIONS
{
    .text : {
        *Setup.o(.text);
        *(.text);
        *(.text.*);
    } > rom
    .init : {
        KEEP (*(.init))
    } > rom

    .fini : {
        KEEP (*(.fini))
    } > rom

    .rodata : {
        __rodata_start = .;
        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r.*)
        __rodata_end = .;
    } > rom

    .sbss : {
        __sbss_start = .;
        *(.sbss)
        *(.sbss.*)
        *(.gnu.linkonce.sb.*)
        __sbss_end = .;
    } > ram

    .data : {
        . = ALIGN(4);
        __data_start = .;
        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d.*)
        __data_end = .;
    } > ram

    .bss : {
        . = ALIGN(32);
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        __bss_end = .;
    } > ram

    .heap : {
        . = ALIGN(4);
        _heap_start = .;
        . += _HEAP_SIZE;
        _heap_end = .;
    } > ram

    .stack : {
        . = ALIGN(32);
        _stack_end = .;
        . += _STACK_SIZE;
        . = ALIGN(4);
        _stack = .;
        __stack = _stack;
    } > ram

    /* . = ORIGIN(ram) + LENGTH(ram) - 4;
    . = ALIGN(0x1000)
    endkernel = .;
    . += _PHY_SIZE;
    endphyspace = .; */

}



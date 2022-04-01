SECTIONS
{
    . = 0x60000;
    PROVIDE(_code = .);
    .text :
    {
        KEEP(*(.text.boot))
    }
    .rodata :
    {
        *(.rodata)
    }
    PROVIDE(_data = .);
    .bss (NOLOAD) :
    {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
    _end = .;
}
__bss_size = (__bss_end - __bss_start)>>3;
__loader_size = (_end - _code)>>3;

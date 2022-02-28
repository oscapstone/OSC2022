ENTRY(_start)
MEMORY
{
    CODE (rx) : ORIGIN = 0x80000, LENGTH = 128K
    RO (r) : ORIGIN = 0xa0000, LENGTH = 128K
    DATA (rw) : ORIGIN = 0x100000, LENGTH = 512K
    RAM (rw) : ORIGIN = 0x180000, LENGTH = 512K
}

SECTIONS
{
    .text : { 
        *(.text.start)
        *(.text)
    } >CODE
    .rodata : {
        *(.rodata)
    } >RO
    .data : {
        *(.data)
    } >DATA
    .bss : {
        _bss_start = .;
        *(.bss)
    } >DATA
    _bss_end = .;
    _stack_end = ORIGIN(RAM) + LENGTH(RAM);
}
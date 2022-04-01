ENTRY(_start)
MEMORY
{
    CODE (rx) : ORIGIN = 0x80000, LENGTH = 128K
    RO (r) : ORIGIN = 0xa0000, LENGTH = 128K
    DATA (rw) : ORIGIN = 0x100000, LENGTH = 512K
    RAM (rw) : ORIGIN = 0x180000, LENGTH = 5M
    USER (rwx) : ORIGIN = 0x680000, LENGTH = 5M
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
    _heap_start = ORIGIN(RAM);
    _heap_end = ORIGIN(RAM) + 4M + 512K;
    _stack_end = ORIGIN(RAM) + LENGTH(RAM);
    _userspace_start = ORIGIN(USER);
    _userspace_end = ORIGIN(USER) + LENGTH(USER) - 8;
}
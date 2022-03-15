
MEMORY
{
    BOOTLOADER (rx) : ORIGIN = 0x10000, LENGTH = 128K
    LOADER (rx) : ORIGIN = 0x80000, LENGTH = 4K
    CODE (rx) : ORIGIN = 0x81000, LENGTH = 124K
    DATA (rw) : ORIGIN = 0xa0000, LENGTH = 128K
    RAM (rw) : ORIGIN = 0x180000, LENGTH = 512K
}

SECTIONS
{
    .text.loader : {
        KEEP(*(.text.loader))
    } >LOADER
    .text : { 
        KEEP(*(.text.start))
        *(.text)
    } >CODE
    .data : {
        *(.rodata)
        *(.data)
    } >DATA
    .bss : {
        _bss_start = .;
        *(.bss)
    } >DATA
    _bss_end = .;
    _stack_end = ORIGIN(RAM) + LENGTH(RAM);
    _bootloader_start = ORIGIN(BOOTLOADER);
    _code_start = ORIGIN(CODE);
    _code_end = ORIGIN(CODE) + LENGTH(CODE);
    _data_start = ORIGIN(DATA);
    _data_end = ORIGIN(DATA) + LENGTH(DATA);
    _kernel_start = ORIGIN(LOADER);
}
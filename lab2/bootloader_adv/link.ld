
SECTIONS{
    . =0x80000;
    __start = .;
    .text : {*(.text)}
    .rodata : {*(.rodata)}
    .data : {*(.data)}
    . = ALIGN(0x16);
    .bss (NOLOAD) :{
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }

    __bss_size = (__bss_end - __bss_start)>>3;
    __end = .;
}
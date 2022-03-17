
SECTIONS{
    . =0x60000;
    __start = .;
    .text : {*(.text)}
    .rodata : {*(.rodata)}
    .data : {*(.data)}
    .bss ALIGN(0x16) :{
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }

    __bss_size = (__bss_end - __bss_start)>>3;
    __end = .;
}
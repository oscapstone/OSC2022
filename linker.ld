SECTIONS
{
    . = 0x80000;
    .text : { *(.text) }
    
    . = ALIGN(16);
    bss_begin = .;
    .bss : { *(.bss*) } 
    bss_end = .;
    
}
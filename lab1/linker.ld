ENTRY(_start) /* set _start in boot.S as entry point */ 

SECTIONS
{
    . = 0x80000; /* bootloader (start.elf) will loader kernel8.img to 0x80000*/
    .text : 
    {
		KEEP(*(.text.boot))
        *(.text)
    }
    .rodata ALIGN(0x8) : 
    {
        *(.rodata)
    }
    .data ALIGN(0x8) : 
    {
        *(.data)
    }

    .bss ALIGN(0x8) : 
    { 
        *(.bss) 
    }
}

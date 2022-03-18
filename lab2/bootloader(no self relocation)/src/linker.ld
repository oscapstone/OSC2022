SECTIONS
{
    . = 0x60000;			/* same with cinfig.txt, because this is a bootloader without self relocation */
    __start = .;
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    . = ALIGN(0x1000);		/* why need align ? */
    .data : { *(.data) }
    . = ALIGN(0x1000);		/* why need align ? */
    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss)
        *(COMMON)
        __bss_end = .;
    }
    . = ALIGN(0x1000);
    .heap : {
    	*(.heap)
    	. += 0x8000;		/* heap size */
    }
    . = ALIGN(0x1000);
    .stack : {
    	*(.stack)
    	. += 0x8000;		/* stack size */
    	__stack_top = .;
    }
    __end = .;
}

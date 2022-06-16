ENTRY(_start) /* set _start in boot.S as entry point */ 

SECTIONS
{
    . = 0xffff000000000000; /* kernel space 0xffff000000000000 ~ 0xffffffffffffffff */
    . += 0x80000; /* bootloader (start.elf) will loader kernel8.img to 0x80000*/
    __kernel_image_start = .;
    __EL1_stack_size = 0x4000;
    __EL1_stack = .;
    __text_start = .;
    .text : 
    {
		KEEP(*(.text.boot))
        *(.text)
    }
    
   . = ALIGN(0x1000);
    __text_end = .;
    __rodata_start = .;
    .rodata : 
    {
        *(.rodata)
    }
    
    . = ALIGN(0x1000);
    __rodata_end = .;
    __data_start = .;
    .data : 
    {
        *(.data)
    }
    
    . = ALIGN(0x1000);
    __data_end = .;
    __bss_start = .;
    .bss : 
    { 
        *(.bss) 
    }

    . = ALIGN(0x1000);
    __bss_end = .;
    __kernel_image_end = .;
    __heap_start = .;

}

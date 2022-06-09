/* entry point */
ENTRY(_start)

_simple_alloc_start = 0xffff00002c000000;
_simple_alloc_end = 0xffff00002dffffff;

SECTIONS
{
    . = 0xffff000000000000;
    . += 0x80000;

    _text_start = .;
    .text : 
    { 
        KEEP(*(.text.boot))
        *(.text) 
    }
    . = ALIGN(0x1000); /* align to page size */
    _text_end = .;

    _data_start = .;
    .data :
    {
        *(.data)
    }
    . = ALIGN(0x1000); /* align to page size */
    _data_end = .;

    _bss_start = .;
    .bss :
    {
        bss = .;
        *(.bss)
    }
    . = ALIGN(0x1000); /* align to page size */
    _bss_end = .;
    _bss_size = _bss_end - _bss_start;

    _heap_start = .;
    . = ALIGN(0x1000);
    _heap_end = .;
}
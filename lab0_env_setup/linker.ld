SECTIONS
{
	. = 0x80000; /* memory location */
	.text : { *(.text) } /* content from input file .text section */
}

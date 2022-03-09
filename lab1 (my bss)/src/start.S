.section ".text"

// let linker.ld start from _start
.global _start

_start:
	wfe
	b 2f

2:
	// set stack pointer
	ldr x0, = 0x200000
	mov sp, x0
	
	// clear bss
	ldr x1, =__bss_start
	ldr x2, =__bss_end
	cmp x1, x2
	blo clear_bss

3:	
	// branch to main function.
	bl main
	
1:
	b 1b
	
clear_bss:
	str wzr, [x1, 4]
	cmp x1, x2
	blo clear_bss
	b 3b

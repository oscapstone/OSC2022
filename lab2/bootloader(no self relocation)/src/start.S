.section ".text"

// let linker.ld start from _start
.global _start

_start:
	// set stack pointer
	ldr x0, = __stack_top
	mov sp, x0
	
	// clear bss
	adr x1, __bss_start
	adr x2, __bss_end
	
	//cmp x1, x2
	//blo clear_bss

3:	
	// branch to main function.
	bl main
	
1:
	wfe
	b 1b
	
//clear_bss:
//	str wzr, [x1, 4]
//	cmp x1, x2
//	blo clear_bss
//	b 3b

.section ".text"
_start:
	mrs	x0, mpidr_el1		
	and	x0, x0,#0xFF		// Check processor id
	cbz	x0, init_rasp

busy_loop:
	wfe
	b busy_loop

init_rasp:
	adr x0,_start
	mov sp,x0
	//load the start & end of bss segment address the initialize them to 0.
	ldr    x0, bss_begin
    ldr   x1, bss_end
    sub    x1, x1, x0 // x1 is the size of the bss segment
    bl     init_bss
init_bss:
	cbz x1,main_loop // if x1==0, jump to infinity loop.
	str xzr, [x0], #8 // assign zero register to the address of x0, then x0=x0-1
	subs x1, x1, #8
	b init_bss

main_loop:
	bl main
	b busy_loop
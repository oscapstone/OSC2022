.section ".text"
.global _start
.global _end

_start:
	mov x15,x0
	mrs	x1, mpidr_el1		
	and	x1, x1,#0xFF		// Check processor id
	cbz	x1, init_rasp

busy_loop:
	wfe
	b busy_loop

init_rasp:
	adr x1,_start
	mov sp,x1
	//load the start & end of bss segment address the initialize them to 0.
	adr    x1, bss_begin
    adr   x2, bss_end
    sub    x2, x2, x1 // x2 is the size of the bss segment
    bl     init_bss
init_bss:
	cbz x2,main_loop // if x2==0, jump to infinity loop.
	str xzr, [x1], #8 // assign zero register to the address of x1, then x1=x1+8
	subs x2, x2, #8
	b init_bss

main_loop:
	bl main
	b busy_loop
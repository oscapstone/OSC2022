// AArch64 ISA general purpose registers:
// 64-bit X register (X0..X30)
// 32-bit W register (W0..W30)

.globl put32
put32:	// Read some data from a 32-bit register.
	str w1,[x0]	// Store contents of w1 to x0
	ret

.globl get32
get32:	// Write some data to and 32-bit register.
	ldr w0,[x0]	// Load from x0 into w0 (w0 is the return register)
	ret

.globl delay
delay:
	subs x0, x0, #1
	bne delay
	ret
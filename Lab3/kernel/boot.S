#include "mm.h"

.section ".text.boot"

.globl _start
_start:	
	mrs	x0, mpidr_el1	// Load value from a system register to one of the general purpose registers (x0–x30)
	and	x0, x0,#0xFF	// Check processor id
	cbz	x0, master		// Hang for all non-primary CPU
	b	proc_hang

proc_hang: 
	b 	proc_hang

master:
	bl from_el2_to_el1

	adr	x0, bss_begin	// Load a label's relative address into the target register
	adr	x1, bss_end		// bss_begin and bss_end are defined in the linker script
	sub	x1, x1, x0		// Sets the condition register for branch
	bl 	memzero			// In ARMv8 architecture, the first seven arguments are passed to the called function via registers x0–x6.

	mov sp, #0x400000
	bl	kernel_main		// Perform an unconditional branch and store the return address in x30
	b 	proc_hang		// Should never come here

from_el2_to_el1:
    mov x0, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x0
    mov x0, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x0
    msr elr_el2, lr
    eret // return to EL1
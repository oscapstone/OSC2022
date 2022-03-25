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

    ldr x0, =exception_table	// setup interrupt vector base
    msr vbar_el1, x0

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



// save general registers to stack
.macro save_regs
    sub sp, sp, 32 * 8
    stp x0, x1, [sp ,16 * 0]
    stp x2, x3, [sp ,16 * 1]
    stp x4, x5, [sp ,16 * 2]
    stp x6, x7, [sp ,16 * 3]
    stp x8, x9, [sp ,16 * 4]
    stp x10, x11, [sp ,16 * 5]
    stp x12, x13, [sp ,16 * 6]
    stp x14, x15, [sp ,16 * 7]
    stp x16, x17, [sp ,16 * 8]
    stp x18, x19, [sp ,16 * 9]
    stp x20, x21, [sp ,16 * 10]
    stp x22, x23, [sp ,16 * 11]
    stp x24, x25, [sp ,16 * 12]
    stp x26, x27, [sp ,16 * 13]
    stp x28, x29, [sp ,16 * 14]
    str x30, [sp, 16 * 15]
.endm

// load general registers from stack
.macro load_regs
    ldp x0, x1, [sp ,16 * 0]
    ldp x2, x3, [sp ,16 * 1]
    ldp x4, x5, [sp ,16 * 2]
    ldp x6, x7, [sp ,16 * 3]
    ldp x8, x9, [sp ,16 * 4]
    ldp x10, x11, [sp ,16 * 5]
    ldp x12, x13, [sp ,16 * 6]
    ldp x14, x15, [sp ,16 * 7]
    ldp x16, x17, [sp ,16 * 8]
    ldp x18, x19, [sp ,16 * 9]
    ldp x20, x21, [sp ,16 * 10]
    ldp x22, x23, [sp ,16 * 11]
    ldp x24, x25, [sp ,16 * 12]
    ldp x26, x27, [sp ,16 * 13]
    ldp x28, x29, [sp ,16 * 14]
    ldr x30, [sp, 16 * 15]
    add sp, sp, 32 * 8
.endm


//exception table
.align 11
exception_table:
	b TODO
	.align 7
	b TODO
	.align 7
	b TODO
	.align 7
	b TODO

	.align 7
	b curr_sync
	.align 7
	b curr_iqr
	.align 7
	b TODO
	.align 7
	b TODO

	.align 7
	b lower_sync
	.align 7
	b lower_iqr
	.align 7
	b TODO
	.align 7
	b TODO

	.align 7
	b TODO
	.align 7
	b TODO
	.align 7
	b TODO
	.align 7
	b TODO

//Synchronous from lower level
lower_sync:
	save_regs
	bl lower_sync_handler
	load_regs
	eret

//IRQ from lower level
lower_iqr:
	save_regs
	bl lower_iqr_handler
	load_regs
	eret

curr_sync:
	save_regs
	bl curr_sync_handler
	load_regs
	eret

curr_iqr:
	save_regs
	bl curr_iqr_handler
	load_regs
	eret

TODO:
	save_regs
	bl error_handler
	load_regs
	eret
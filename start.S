// save general registers to stack
.macro save_all
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
.macro load_all
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

.section ".text.boot"

.global _start

_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // cpu id > 0, stop
1:  wfe
    b       1b
2:  // cpu id == 0

    // loading fdt-address at register x0
    ldr x1, =_fdt_ptr
    str x0, [x1]
    
    // exception level init
    bl from_el2_to_el1
    // the next instruction runs in EL1

    // set exception vector table
	adr x0, exception_vector_table
	msr VBAR_EL1, x0

    // set top of stack just before our code
    ldr     x1, =_start
    mov     sp, x1

    // clear bss
    ldr     x1, =__bss_start
    ldr     x2, =__bss_end
    sub     x2, x2, x1
3:  cbz     x2, 4f
    str     xzr, [x1], #8
    sub     x2, x2, #8
    cbnz    x2, 3b

4:  // jump to C code, should not return
    bl      main
    // for failsafe, halt this core too
    b       1b

from_el2_to_el1:
	mov x0, (1 << 31) // EL1 uses aarch64
	msr hcr_el2, x0
	mov x0, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
	msr spsr_el2, x0
	msr elr_el2, x30//x30 = link register
    // disable SIMD traps: built-ins of uart_printf will use SIMD
	// https://github.com/bztsrc/raspi3-tutorial/tree/master/12_printf
    // http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0500e/CIHBGEAB.html ldr x0, =(1 << 20)
	mov x0, #(3 << 20)
	msr cpacr_el1, x0
	eret // return to EL1

// exception vector table
.align 11
.global exception_vector_table
exception_vector_table:
	// EL1t
	b invalid_handler
	.align 7
	b invalid_handler
	.align 7
	b invalid_handler
	.align 7
	b invalid_handler
	// EL1h
	.align 7
	b invalid_handler
	.align 7
	b invalid_handler
	.align 7
	b invalid_handler
	.align 7
	b invalid_handler
	// 64-bit EL0
	.align 7
	b lower_sync_handler
	.align 7
	b lower_irq_handler
	.align 7
	b invalid_handler
	.align 7
	b invalid_handler
	// 32-bit EL0
	.align 7
	b invalid_handler
	.align 7
	b invalid_handler
	.align 7
	b invalid_handler
	.align 7
	b invalid_handler

//Synchronous from lower level
lower_sync_handler:
	save_all
	bl lower_sync_entry
	load_all
	eret

//IRQ from lower level
lower_irq_handler:
	save_all
	bl lower_irq_entry
	load_all
	eret

invalid_handler:
	save_all
	bl invalid_entry
	load_all
	eret
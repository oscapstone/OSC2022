    .macro	ventry	label
	.align	7
	b	\label
	.endm


    .macro	save_all
    sub sp, sp, 17 * 16
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

    mrs x24, spsr_el1
    mrs x25, elr_el1
    stp x24, x25, [sp, 16 * 16]
	.endm

    .macro load_all
    ldp x24, x25, [sp, 16 * 16]
    msr spsr_el1, x24 
    msr elr_el1, x25 

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
    add sp, sp, 17 * 16
    .endm


// exception vector table 

.align 11
.global EL1_exception_vector_table
EL1_exception_vector_table:
// Exception from the current EL while using SP_EL0 
    ventry	invalid_handler  		// Synchronous EL1t
	ventry	invalid_handler			// IRQ EL1t
	ventry	invalid_handler			// FIQ EL1t
	ventry	invalid_handler	    	// Error EL1t

// Exception from the current EL while using SP_ELx 
	ventry	el1_sync_handler        // Synchronous EL1h
	ventry	el1_irq_handler         // IRQ EL1h
	ventry	invalid_handler   	    // FIQ EL1h
	ventry	invalid_handler		    // Error EL1h

// Exception from a lower EL at least one lower EL is AArch64 
	ventry	el0_sync_handler        // Synchronous 64-bit EL0
	ventry	el0_irq_handler         // IRQ 64-bit EL0
	ventry	invalid_handler   	    // FIQ 64-bit EL0
	ventry	invalid_handler		    // Error 64-bit EL0

// Exception from a lower EL and all lower ELs are AArch32 
	ventry	invalid_handler   	    // Synchronous 32-bit EL0
	ventry	invalid_handler   	    // IRQ 32-bit EL0
	ventry	invalid_handler   	    // FIQ 32-bit EL0
	ventry	invalid_handler		    // Error 32-bit EL0



invalid_handler:
el1_sync_handler:
    save_all
    nop
    load_all
    eret


el1_irq_handler:
el0_irq_handler:
    save_all
    bl irq_handler
    load_all
    eret

el0_sync_handler:
    save_all
    mrs x24, esr_el1
    lsr x24, x24, #26
    cmp x24, #0x15

    b.eq svc_handler
    b    nothing

svc_handler:
    adr x24, syscall_table
    ldr x24, [x24]

    blr x24

    str x0, [sp]
    load_all
    eret

nothing:
    load_all 
    eret

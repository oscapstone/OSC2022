// save general registers to stack
.macro save_all
    sub sp, sp, 32 * 9
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
    // nested interrupt
    mrs x0,  spsr_el1
    str x0,  [sp, 16 * 15 + 8]
    mrs x0,  elr_el1
    str x0,  [sp, 16 * 16]
    mrs x0,  sp_el0
    str x0,  [sp, 16 * 16 + 8]

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
    // nested interrupt
    ldr x0, [sp, 16 * 15 + 8]
    msr spsr_el1, x0
    ldr x0,  [sp, 16 * 16]
    msr elr_el1, x0
    ldr x0,  [sp, 16 * 16 + 8]
    msr sp_el0, x0
    // re-ldp x0
    ldp x0, x1, [sp ,16 * 0]
    add sp, sp, 32 * 9
.endm

.section ".text.boot"

.global _start

_start:
    // store fdt-address at register x9
    mov x9, x0
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // cpu id > 0, stop
1:  wfe
    b       1b
2:  // cpu id == 0
   
    // exception level init
    bl from_el2_to_el1
    // the next instruction runs in EL1
    
    // set MMU identity paging
    bl identity_paging

    // set exception vector table
	adr x0, exception_vector_table
	msr VBAR_EL1, x0

    // set top of stack just before our code
    ldr     x1, =0xFFFF00003C000000
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
	b current_irq_handler
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
    mov x0, sp  // trap_frame
	bl lower_sync_entry
	load_all
	eret

//IRQ from lower level
lower_irq_handler:
	save_all
    mov x0, sp  // trap_frame
	bl schedule_irq
	load_all
	eret

invalid_handler:
	save_all
	bl invalid_entry
	load_all
	eret

current_irq_handler:
	save_all
    mov x0, sp  // trap_frame
	bl schedule_irq
	load_all
	eret

.global switch_to
switch_to:
    stp x19, x20, [x0, 16 * 0]
    stp x21, x22, [x0, 16 * 1]
    stp x23, x24, [x0, 16 * 2]
    stp x25, x26, [x0, 16 * 3]
    stp x27, x28, [x0, 16 * 4]
    stp fp, lr, [x0, 16 * 5]
    mov x9, sp
    str x9, [x0, 16 * 6]
    mrs x9, spsr_el1
    str x9, [x0, 16 * 6 + 8]

    ldp x19, x20, [x1, 16 * 0]
    ldp x21, x22, [x1, 16 * 1]
    ldp x23, x24, [x1, 16 * 2]
    ldp x25, x26, [x1, 16 * 3]
    ldp x27, x28, [x1, 16 * 4]
    ldp fp, lr, [x1, 16 * 5]
    ldr x9, [x1, 16 * 6]
    mov sp,  x9
    ldr x9, [x1, 16 * 6 + 8]
    msr spsr_el1, x9
    msr tpidr_el1, x1
    ret

.global store_context
store_context:
    stp x19, x20, [x0, 16 * 0]
    stp x21, x22, [x0, 16 * 1]
    stp x23, x24, [x0, 16 * 2]
    stp x25, x26, [x0, 16 * 3]
    stp x27, x28, [x0, 16 * 4]
    stp fp, lr, [x0, 16 * 5]
    mov x9, sp
    str x9, [x0, 16 * 6]
    mrs x9, spsr_el1
    str x9, [x0, 16 * 6 + 8]
    ret

.global load_context
load_context:
    ldp x19, x20, [x0, 16 * 0]
    ldp x21, x22, [x0, 16 * 1]
    ldp x23, x24, [x0, 16 * 2]
    ldp x25, x26, [x0, 16 * 3]
    ldp x27, x28, [x0, 16 * 4]
    ldp fp, lr, [x0, 16 * 5]
    ldr x9, [x0, 16 * 6]
    mov sp,  x9
    ldr x9, [x0, 16 * 6 + 8]
    msr spsr_el1, x9
    ret

.globl get_el
get_el:
	mrs x0, CurrentEL
	lsr x0, x0, #2
	ret

.global get_current
get_current:
    mrs x0, tpidr_el1
    ret

#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)
#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1
#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_PAGE 0b11
#define PD_ACCESS (1 << 10)
#define USER_RW ((1 << 6) | (0 << 7))
#define BOOT_PGD_ATTR PD_TABLE
#define BOOT_PUD_ATTR PD_TABLE

#define PD_RAM_ATTR (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK)
#define USER_READ_WRITE (PD_ACCESS | USER_RW | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK)
#define PD_PERIPHERAL_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)
#define PD_RAM (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_PAGE)
#define PD_OUTPUT_MASK 0x0000fffffffff000

identity_paging:
    ldr x0, = TCR_CONFIG_DEFAULT
    msr tcr_el1, x0

    ldr x0, =( \
    (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
    (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) \
    )
    msr mair_el1, x0

    mov x0, 0x0000 // PGD's page frame at 0x1000
    mov x1, 0x2000 // PUD's page frame at 0x2000

    // PGD's PUD
    ldr x2, = BOOT_PGD_ATTR
    orr x2, x1, x2 // combine the physical address of next level page with attribute.
    str x2, [x0]

    // PUD's PMD
    ldr x2, = BOOT_PUD_ATTR
    mov x3, 0x3000 // PMD's page frame at 0x3000
    orr x3, x2, x3
    str x3, [x1]

    // PUD's 0x40000000 ~ 0x7FFFFFFF
    ldr x2, = PD_PERIPHERAL_ATTR
    mov x3, 0x40000000
    orr x3, x2, x3
    str x3, [x1, 8] // 2nd 1GB mapped by the 2nd entry of PUD

    // PMD's 0x00000000 ~ 0x3F000000
    mov x1, 0x3000 // PMD's page frame at 0x3000
    ldr x10, = PD_RAM_ATTR
    mov x2, 0x00000000
    mov x4, #1
5:  orr x3, x10, x2
    str x3, [x1] // 2nd 1GB mapped by the 2nd entry of PUD
    add x1, x1, #8
    add x2, x2, #0x200000
    sub x4, x4, #1
    cbnz x4, 5b

    // PMD's 0x00000000 ~ 0x3F000000
    ldr x10, = USER_READ_WRITE
    mov x4, #478
6:  orr x3, x10, x2
    str x3, [x1] // 2nd 1GB mapped by the 2nd entry of PUD
    add x1, x1, #8
    add x2, x2, #0x200000
    sub x4, x4, #1
    cbnz x4, 6b

    // PMD's 0x00000000 ~ 0x3F000000
    ldr x10, = PD_RAM_ATTR
    mov x4, #1
7:  orr x3, x10, x2
    str x3, [x1] // 2nd 1GB mapped by the 2nd entry of PUD
    add x1, x1, #8
    add x2, x2, #0x200000
    sub x4, x4, #1
    cbnz x4, 7b

    // PMD's 0x00000000 ~ 0x3F000000
    ldr x10, = PD_RAM_ATTR
    mov x4, #24
8:  orr x3, x10, x2
    str x3, [x1] // 2nd 1GB mapped by the 2nd entry of PUD
    add x1, x1, #8
    add x2, x2, #0x200000
    sub x4, x4, #1
    cbnz x4, 8b

    // PMD's 0x3F000000 ~ 0x3FFFFFFF
    ldr x10, = PD_PERIPHERAL_ATTR
    mov x4, #8
9:  orr x3, x10, x2
    str x3, [x1]
    add x1, x1, #8
    add x2, x2, #0x200000
    sub x4, x4, #1
    cbnz x4, 9b

    mov x3, 0x5000 // PGD's page frame at 0x1000
    mov x1, 0x6000 // PUD's page frame at 0x2000
    // PGD's PUD
    ldr x2, = BOOT_PGD_ATTR
    orr x2, x1, x2 // combine the physical address of next level page with attribute.
    str x2, [x3]

    // PUD's PMD
    ldr x2, = BOOT_PUD_ATTR
    mov x3, 0x7000 // PMD's page frame at 0x3000
    orr x3, x2, x3
    str x3, [x1]

    // PMD's 0x00000000 ~ 0x3F000000
    mov x1, 0x7000 // PMD's page frame at 0x3000
    ldr x10, = USER_READ_WRITE
    mov x2, 0x3C000000
    mov x4, #24
10: orr x3, x10, x2
    str x3, [x1, 480*8] // 2nd 1GB mapped by the 2nd entry of PUD
    add x1, x1, #8
    add x2, x2, #0x200000
    sub x4, x4, #1
    cbnz x4, 10b

    mov x1, 0x5000
    msr ttbr0_el1, x0 // load PGD to the bottom translation-based register.
    msr ttbr1_el1, x0 // also load PGD to the upper translation based register.

    mrs x2, sctlr_el1
    orr x2 , x2, 1
    msr sctlr_el1, x2 // enable MMU, cache remains disabled

    dsb ish 
    isb
  
    ldr x0, =0xffff000000000000
    add x30, x30, x0
	ret
.section ".text"
.align 11 // vector table should be aligned to 0x800
.global exception_vector_table

exception_vector_table:
  b exception_handler0 // branch to a handler function.
  .align 7 // entry size is 0x80, .align will pad 0
  b exception_handler1
  .align 7
  b exception_handler2
  .align 7
  b exception_handler3
  .align 7

  b exception_handler0 // branch to a handler function.
  .align 7 // entry size is 0x80, .align will pad 0
  b exception_handler1
  .align 7
  b exception_handler2
  .align 7
  b exception_handler3
  .align 7

  b exception_handler0 // branch to a handler function.
  .align 7 // entry size is 0x80, .align will pad 0
  b exception_handler1
  .align 7
  b exception_handler2
  .align 7
  b exception_handler3
  .align 7

  b exception_handler0 // branch to a handler function.
  .align 7 // entry size is 0x80, .align will pad 0
  b exception_handler1
  .align 7
  b exception_handler2
  .align 7
  b exception_handler3
  .align 7


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
    
    //using for nested interrupt
    mrs x0, spsr_el1
    mrs x1, elr_el1
    stp x0, x1, [sp, 16 * 16]
    ldp x0, x1, [sp ,16 * 0]  // restore x0
.endm

// load general registers from stack
.macro load_all
    // ldp x0, x1, [sp ,16 * 0]
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
    
    ldp x0, x1, [sp ,16 * 16]
    msr spsr_el1, x0
    msr elr_el1, x1

    ldp x0, x1, [sp ,16 * 0]
    add sp, sp, 32 * 9
.endm

exception_handler0: // for system call (svc)
    save_all
    mrs x1, esr_el1
    mrs x2, elr_el1 // return address
    mrs x3, spsr_el1 // processor state1
    mov x4, #0 
    bl exc_dump    
    load_all
    eret

exception_handler1:
    save_all
    mrs x1, esr_el1
    mrs x2, elr_el1 // return address
    mrs x3, spsr_el1 // processor state
    mov x4, #1
    // bl exc_dump
    bl irq_dump
    load_all
    eret

exception_handler2:
    save_all
    mrs x1, esr_el1
    mrs x2, elr_el1 // return address
    mrs x3, spsr_el1 // processor state
    mov x4, #2
    bl exc_dump
    load_all
    eret

exception_handler3:
    save_all
    mrs x1, esr_el1
    mrs x2, elr_el1 // return address
    mrs x3, spsr_el1 // processor state
    mov x4, #3
    bl exc_dump
    load_all
    eret
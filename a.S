.section ".text"
.global _start

_start:
	mov x15,x0
	mrs	x0, mpidr_el1		
	and	x0, x0,#0xFF		// Check processor id
	cbz	x0, init_rasp

busy_loop:
	wfe
	b busy_loop

init_rasp:
	bl from_el2_to_el1
    bl set_exception_vector_table
	adr x0,_start
	mov sp,x0
	//load the start & end of bss segment address the initialize them to 0.
	adr    x0, bss_begin
    adr   x1, bss_end
    sub    x1, x1, x0 // x1 is the size of the bss segment
    bl     init_bss
from_el2_to_el1:
    mov x0, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x0 // Hypervisor Configuration Register, set RW to 1.
    mov x0, 0x5 // EL1h (SPSel = 1) with interrupt disabled(0x3c5), enable(0x5)
    msr spsr_el2, x0
    msr elr_el2, lr
    eret // return to EL1
init_bss:
	cbz x1,main_loop // if x1==0, jump to infinity loop.
	str xzr, [x0], #8 // assign zero register to the address of x0, then x0=x0+8
	subs x1, x1, #8
	b init_bss
.global from_el1_to_el0
from_el1_to_el0:
    mov x0,0
    msr spsr_el1, x0
    msr elr_el1, lr
    eret

main_loop:
    // bl from_el1_to_el0
    msr DAIFClr, 0xf
    bl core_timer_enable
    //mov x0, 0
    //msr cntp_ctl_el0, x0 // enable
	bl main
	b busy_loop

.macro save_all
    sub sp, sp, 16 * 17
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
    
    mrs x9, sp_el0
    stp x30,x9,  [sp, 16 * 15]
    // for nested interrupt
    mrs x9, spsr_el1
    mrs x10, elr_el1
    stp x9,x10, [sp, 16 * 16]

    // ldp x0, x1, [sp ,16 * 0]

    // for el0 user stack pointer
    

.endm

// load general registers from stack
.macro load_all

    ldp x30,x9, [sp, 16 * 15]
    msr sp_el0,x9

    ldp x9,x10,[sp, 16 * 16]
    msr spsr_el1,x9
    msr elr_el1,x10

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
    // ldr x30, [sp, 16 * 15]

    add sp, sp, 16 * 17
.endm

.global load_eret
load_eret:
    load_all
    eret

exception_handler:
    save_all
    mov x0,sp
    bl exception_entry
    load_all
    eret

irq_handler:
    save_all
    mov x0,sp
    bl irq_entry
    load_all
    eret

.align 11 // vector table should be aligned to 0x800
.global exception_vector_table
exception_vector_table:
  b exception_handler // branch to a handler function.
  .align 7 // entry size is 0x80, .align will pad 0
  b exception_handler
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7

  b exception_handler
  .align 7
  b irq_handler
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7

  b exception_handler
  .align 7
  b irq_handler
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7

  b exception_handler
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7

set_exception_vector_table:
  adr x0, exception_vector_table
  msr vbar_el1, x0
  ret
// save general registers to stack

#define CORE0_TIMER_IRQ_CTRL 0x40000040
.global core_timer_enable
.global core_timer_disable
.global core_timer_handler
core_timer_enable:
    mov x0, 1
    msr cntp_ctl_el0, x0 // enable
    mrs x0, cntfrq_el0
    add x0,x0,x0
    //mov x1,0xffffffff
    //mul x0,x0,x1
    msr cntp_tval_el0, x0 // set expired time
    mov x0, 2
    ldr x1, =CORE0_TIMER_IRQ_CTRL
    str w0, [x1] // unmask timer interrupt
    ret
core_timer_disable:
    mov x0, 0
    msr cntp_ctl_el0, x0 // enable

   // mov x0, 2
   // ldr x1, =CORE0_TIMER_IRQ_CTRL
   // str w0, [x1] // unmask timer interrupt
    ret
core_timer_handler:
  mrs x0, cntfrq_el0
  //mrs x1, cntp_ctl_el0
  add x0,x0,x0
  // add x0,x0,x1
  msr cntp_tval_el0, x0
  ret

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

    ldp x19, x20, [x1, 16 * 0]
    ldp x21, x22, [x1, 16 * 1]
    ldp x23, x24, [x1, 16 * 2]
    ldp x25, x26, [x1, 16 * 3]
    ldp x27, x28, [x1, 16 * 4]
    ldp fp, lr, [x1, 16 * 5]
    ldr x9, [x1, 16 * 6]
    mov sp,  x9
    msr tpidr_el1, x1
    ret

.global get_current
get_current:
    mrs x0, tpidr_el1
    ret

.global save_cpu_context
save_cpu_context:
    stp x19, x20, [x0, 16 * 0]
    stp x21, x22, [x0, 16 * 1]
    stp x23, x24, [x0, 16 * 2]
    stp x25, x26, [x0, 16 * 3]
    stp x27, x28, [x0, 16 * 4]
    stp fp, lr, [x0, 16 * 5]
    mov x9, sp
    str x9, [x0, 16 * 6]
    ret

.global load_cpu_context
load_cpu_context:
    ldp x19, x20, [x0, 16 * 0]
    ldp x21, x22, [x0, 16 * 1]
    ldp x23, x24, [x0, 16 * 2]
    ldp x25, x26, [x0, 16 * 3]
    ldp x27, x28, [x0, 16 * 4]
    ldp fp, lr, [x0, 16 * 5]
    ldr x9, [x0, 16 * 6]
    mov sp,  x9
    ret

.global get_pid
get_pid:
    mov x8,0
    svc #0
    ret

.global uart_read
uart_read:
    mov x8,1
    svc #0
    ret

.global uart_write
uart_write:
    mov x8,2
    svc #0
    ret

.global exec
exec:
    mov x8,3
    svc #0
    ret

.global fork
fork:
    mov x8,4
    svc #0
    ret

.global exit
exit:
    mov x8,5
    svc #0
    ret

.global mbox_call
mbox_call:
    mov x8,6
    svc #0
    ret

.global kill
kill:
    mov x8,7
    svc #0
    ret
.global ls
ls:
    mov x8,10
    svc #0
    ret
.global open
open:
    mov x8,11
    svc #0
    ret
.global close
close:
    mov x8,12
    svc #0
    ret
.global write
write:
    mov x8,13
    svc #0
    ret
.global read
read:
    mov x8,14
    svc #0
    ret
.global mkdir
mkdir:
    mov x8,15
    svc #0
    ret
.global mount
mount:
    mov x8,16
    svc #0
    ret
.global chdir
chdir:
    mov x8,17
    svc #0
    ret
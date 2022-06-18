
.section ".text.boot"

.global _start
// #include "mmu.h"
#define CORE0_TIMER_IRQ_CTRL 0xFFFF000040000040
#define CORE0_IRQ_SOURCE     0xFFFF000040000060
#define PAGE_TABLE_BASE            (0x30000000)
#define PGD_BASE                   (PAGE_TABLE_BASE + 0x0000)
#define PUD_BASE                   (PAGE_TABLE_BASE + 0x1000)
#define PMD_BASE                   (PAGE_TABLE_BASE + 0x2000)
#define PTE_BASE                   (PAGE_TABLE_BASE + 0x4000)

#define TCR_CONFIG_REGION_48bit    (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB             ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT         (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE         (0b00000000)
#define MAIR_NORMAL_NOCACHE        (0b01000100)
#define MAIR_CONFIG                (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8))
#define MAIR_IDX_DEVICE_nGnRnE     (0)
#define MAIR_IDX_NORMAL_NOCACHE    (1)

#define PD_TABLE                   (0b11)
#define PD_BLOCK                   (0b01)
#define PD_PAGE                    (0b11)
#define PD_ACCESS                  (1 << 10)
#define PD_WRITE                   (1 << 7)
#define PD_USER                    (1 << 6)

#define BOOT_PGD_ATTR              PD_TABLE
#define BOOT_PUD_ATTR              PD_TABLE
#define BOOT_PMD_ATTR              PD_TABLE
#define BOOT_PTE_RAM_ATTR          (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_PAGE)
#define BOOT_PTE_DEVICE_ATTR       (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2)  | PD_PAGE)

#define PD_RAM_ATTR                (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK)
#define PD_USER_ATTR               (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK | PD_USER) 

#define PERIPHERAL_BASE            (0x3C000000)
#define PERIPHERAL_END             (0x3F000000)


_start:
    mov     x10, x0
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, cpu_id_0
    // cpu id > 0, stop
1:  wfe
    b       1b
cpu_id_0:  // cpu id == 0
    bl      from_el2_to_el1  // change level from el2 to el1
    bl      setup_vmem

set_exception_vector_table:
    adr x1, exception_vector_table // adr: Generates a PC-relative address
    msr vbar_el1, x1

    // core_timer_enable
    mov x1, 1
    msr cntp_ctl_el0, x1 // enable timer, umask time irq
    mrs x1, cntfrq_el0
    msr cntp_tval_el0, x1 // set expired time
    mov x1, 2
    ldr x2, =CORE0_TIMER_IRQ_CTRL
    str w1, [x2] // enable timer interrupt

    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
    ldr     x1, =_start
    mov     sp, x1

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

    // jump to C code, should not return
4:
    mov     x0, x10
    ldr     x1, =0xffff000000000000
    add     x0, x0, x1
    bl      main
    // for failsafe, halt this core too
    b       1b

from_el2_to_el1:
    mov     x5, (1 << 31)   // EL1 uses aarch64
    msr     hcr_el2, x5     // Hypervisor Configuration Register
    mov     x5, 0x345       // EL1h (SPSel = 1) with interrupt enabled

    msr     spsr_el2, x5    // spsr is saved program status register. "bits[3:0] == 5" means EL1h. "bits[4] == 0" means AArch64.
                            // "bits[6]" means FIQ interrupt mask. "bits[7]" means IRQ interrupt mask.
                            // "bits[8]" means SError interrupt mask. "bits[9]" Debug exception mask.

    msr     elr_el2, lr     // lr is link register. For now, it points to address of "adr x1, exception_vector_table" 
    eret                    // return to EL1



// save general registers to stack
.macro save_all
    sub     sp, sp, 16 * 18
    // saving general registers
    stp     x0, x1, [sp ,16 * 0]
    stp     x2, x3, [sp ,16 * 1]
    stp     x4, x5, [sp ,16 * 2]
    stp     x6, x7, [sp ,16 * 3]
    stp     x8, x9, [sp ,16 * 4]
    stp     x10, x11, [sp ,16 * 5]
    stp     x12, x13, [sp ,16 * 6]
    stp     x14, x15, [sp ,16 * 7]
    stp     x16, x17, [sp ,16 * 8]
    stp     x18, x19, [sp ,16 * 9]
    stp     x20, x21, [sp ,16 * 10]
    stp     x22, x23, [sp ,16 * 11]
    stp     x24, x25, [sp ,16 * 12]
    stp     x26, x27, [sp ,16 * 13]
    stp     x28, x29, [sp ,16 * 14]
    str     x30, [sp ,16 * 15]

    // saving el registers
    mrs     x0, SPSR_EL1
    str     x0, [sp, 16 * 15 + 8]
    mrs     x0, ELR_EL1
    mrs     x1, SP_EL0
    stp     x0, x1, [sp, 16 * 16]
.endm

// load general registers from stack
.macro load_all
    // load el registers
    ldr     x0, [sp, 16 * 15 + 8]
    msr     SPSR_EL1, x0
    ldp     x0, x1, [sp, 16 * 16]
    msr     ELR_EL1, x0
    msr     SP_EL0, x1

    // load general registers
    ldp     x0, x1, [sp ,16 * 0]
    ldp     x2, x3, [sp ,16 * 1]
    ldp     x4, x5, [sp ,16 * 2]
    ldp     x6, x7, [sp ,16 * 3]
    ldp     x8, x9, [sp ,16 * 4]
    ldp     x10, x11, [sp ,16 * 5]
    ldp     x12, x13, [sp ,16 * 6]
    ldp     x14, x15, [sp ,16 * 7]
    ldp     x16, x17, [sp ,16 * 8]
    ldp     x18, x19, [sp ,16 * 9]
    ldp     x20, x21, [sp ,16 * 10]
    ldp     x22, x23, [sp ,16 * 11]
    ldp     x24, x25, [sp ,16 * 12]
    ldp     x26, x27, [sp ,16 * 13]
    ldp     x28, x29, [sp ,16 * 14]
    ldr     x30, [sp, 16 * 15]

    add     sp, sp, 16 * 18
.endm

core_timer_handler:
    bl print_sec
    mrs x1, cntfrq_el0    // set next expired time
    lsr x1, x1, #5        // two seconds
    msr cntp_tval_el0, x1 // On a write of this register, CNTP_CVAL_EL0 is set to (CNTPCT_EL0 + TimerValue).
    load_all
    eret

exception_handler:
    save_all
    mov x0, sp
    bl svc_handler
    load_all
    eret

gpu_irq_handler:
    bl uart_irq_handler
    load_all
    eret

kernel_irq_source_determine:
    save_all
    ldr x2, =CORE0_IRQ_SOURCE
    ldr x3, [x2]
    and x4, x3, #256  // this position is set due to GPU irq
    cmp x4, #256
    beq gpu_irq_handler

    and x4, x3, #2    // this position is set due to CNTPNSIRQ irq
    cmp x4, #2
    beq core_timer_handler

    bl exit_280  // other irq not handle

usr_irq_source_determine:
    save_all
    //bl enable_uart_irq
    ldr x2, =CORE0_IRQ_SOURCE
    ldr x3, [x2]

    and x4, x3, #2
    cmp x4, #2
    //bl exit_480
    beq core_timer_handler

    bl exit_480  // other irq not handle



.align 11      // vector table should be aligned to 0x800
.global exception_vector_table
exception_vector_table:
  b exit_000       // branch to a handler function.
  .align 7     // entry size is 0x80, .align will pad 0
  b exit_080
  .align 7
  b exit_100
  .align 7
  b exit_180
  .align 7

  b exit_200
  .align 7
  b kernel_irq_source_determine // irq handler
  .align 7
  b exit_300
  .align 7
  b exit_380
  .align 7

  b exception_handler // svc 0, Exception from a lower EL and at least one lower EL is AARCH64, Synchronnous
  .align 7
  b usr_irq_source_determine
  .align 7
  b exit_500
  .align 7
  b exit_580
  .align 7

  b exit
  .align 7
  b exit
  .align 7
  b exit
  .align 7
  b exit
  .align 7


.global switch_to
switch_to:
    stp x19, x20, [x0, 16 * 0]
    stp x21, x22, [x0, 16 * 1]
    stp x23, x24, [x0, 16 * 2]
    stp x25, x26, [x0, 16 * 3]
    stp x27, x28, [x0, 16 * 4]
    stp  fp,  lr, [x0, 16 * 5]
    mov  x9,  sp
    str  x9,      [x0, 16 * 6]

    ldp x19, x20, [x1, 16 * 0]
    ldp x21, x22, [x1, 16 * 1]
    ldp x23, x24, [x1, 16 * 2]
    ldp x25, x26, [x1, 16 * 3]
    ldp x27, x28, [x1, 16 * 4]
    ldp  fp,  lr, [x1, 16 * 5]
    ldr  x9,      [x1, 16 * 6]
    mov  sp,  x9
    msr tpidr_el1, x2

    dsb ish                              // ensure write has completed
    msr ttbr0_el1, x3                    // switch translation based address.
    tlbi vmalle1is                       // invalidate all TLB entries
    dsb ish                              // ensure completion of TLB invalidatation
    isb                                  // clear pipeline

    ret


.global from_EL1_to_EL0
from_EL1_to_EL0:
    msr     elr_el1, x0
    msr     sp_el0, x1
    mov     sp, x2
    mov     x1, 0x340
    msr     spsr_el1, x1
    eret

.global store_context
store_context:
    stp x19, x20, [x0, 16 * 0]
    stp x21, x22, [x0, 16 * 1]
    stp x23, x24, [x0, 16 * 2]
    stp x25, x26, [x0, 16 * 3]
    stp x27, x28, [x0, 16 * 4]
    stp  fp,  lr, [x0, 16 * 5]
    mov  x9,  sp
    str  x9,      [x0, 16 * 6]
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
    ret

.global sys_get_pid
sys_get_pid:
    mov x8, 0
    svc 0
    ret

.global sys_fork
sys_fork:
    mov x8, 4
    svc 0
    ret

.global sys_exit
sys_exit:
    mov x8, 5
    svc 0
    ret

.global sys_ret
sys_ret:
    mov x8, 10
    svc 0
    ret

setup_vmem:
    ldr x1, =TCR_CONFIG_DEFAULT
    msr tcr_el1, x1

    ldr x1, =MAIR_CONFIG
    msr mair_el1, x1

    ldr x0, =PGD_BASE // PGD's page frame addr
    ldr x1, =PUD_BASE // PUD's page frame addr
    ldr x2, =PMD_BASE // PMD's page frame addr, have two PMD table
    ldr x3, =PTE_BASE // PTE's page frame addr
    
    msr ttbr0_el1, x0 // load PGD to the bottom translation-based register.
    msr ttbr1_el1, x0 // also load PGD to the upper translation based register.

    // set PGD
    ldr x4, =BOOT_PGD_ATTR
    orr x4, x1, x4                      // combine the physical address of next level page with attribute.
    str x4, [x0]

    // set 1st entry of PUD 
    ldr x4, =BOOT_PUD_ATTR
    mov x5, x2
    orr x6, x5, x4                      // combine the physical address of next level page with attribute.
    str x6, [x1]
    // set 2nd entry of PUD 
    add x5, x5, #0x1000
    orr x6, x5, x4
    str x6, [x1, 8]

    // set PMD
    ldr x4, =BOOT_PMD_ATTR
    mov x5, x2                          // addr of PMD
    mov x6, x3                          // addr of PTE
    mov x7, #(512 * 2)                  // 2 PMD table, 1024 entries
setup_pmd:
    orr x8, x6, x4                      // PTE table | ATTR
    str x8, [x5]
    add x5, x5, #8                      // next PMD entry
    add x6, x6, #0x1000                 // next PTE table
    sub x7, x7, #1
    cbnz x7, setup_pmd

    // set PTE
    ldr x4, =BOOT_PTE_RAM_ATTR
    mov x5, #0x00000000
    mov x6, x3                          // addr of PTE
    mov x7, #(1024 * 512)
    ldr x8, =PERIPHERAL_BASE
    
setup_pte_loop:
    cmp x5, x8
    blt setup_pte
    ldr x4, =BOOT_PTE_DEVICE_ATTR

setup_pte:
    orr x9, x5 ,x4
    str x9, [x6]                         // 4KB page
    add x6, x6, #8                       // next PTE entry
    add x5, x5, #4096                    // next phy page
    sub x7, x7, #1
    cbnz x7, setup_pte_loop

    // Enable MMU
    mrs x2, sctlr_el1
    orr x2 , x2, 1
    msr sctlr_el1, x2                    // enable MMU, cache remains disabled

    ldr x2, =set_exception_vector_table  // indirect branch to the virtual address
    br  x2

.global switch_pgd
switch_pgd:
    dsb ish                              // ensure write has completed
    msr ttbr0_el1, x0                    // switch translation based address.
    tlbi vmalle1is                       // invalidate all TLB entries
    dsb ish                              // ensure completion of TLB invalidatation
    isb                                  // clear pipeline
    ret
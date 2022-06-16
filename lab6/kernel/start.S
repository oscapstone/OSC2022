#include "mm/pgtable.h"
.section ".text.boot"
.global _start
_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #0xff

    // cpu id > 0, stop
    cbz     x1, master_core 

proc_hang:  
	wfe
    b       proc_hang

// cpu id == 0
master_core:
    adr     x1, _dtb
    str     x0, [x1]
// initialize .bss section
    adr     x1, __bss_start
    adr     x2, __bss_end
bss_zero:
    sub     x3, x2, x1
    cbz     x3, end 
    str     xzr, [x1], #8 // store zero to [x1] and add 8 to x1
    b bss_zero

end:
// modify vbar_el1 before changing exception level to 1 (in virtual address)
    ldr x0, =EL1_exception_vector_table
    msr vbar_el1, x0

// set up identity pageing
    bl identity_paging

// from el2 to el1
    bl from_el2_to_el1

// enable virtual memory
    ldr x2, =vm_enable
    br x2 

vm_enable:
// kernel_main(_dtb)
    ldr     x1, =_dtb
    ldr     x0, [x1]

// set tpidr_el1 to NULL to avoid timer interrupt accidently update current task_strcut
    mov x1, 0
    msr tpidr_el1, x1


// jump to kernel_main
    bl      kernel_main
    b       proc_hang

from_el2_to_el1:
    ldr x0, =__EL1_stack // Set el1 stack to __EL1_stack
    msr sp_el1, x0

    mov x0, (1 << 31) // Set hcr_el2.RW so EL1 uses aarch64
    msr hcr_el2, x0

    mov x0, 0x3c5 // EL1h (SPSel = 1)
    msr spsr_el2, x0
    msr elr_el2, lr
    eret // return to EL1

identity_paging:
// set page granularity ofttbr0 and ttbr1 to 4KiB and set virtual memory mapping region to 0 ~ 2^48
    ldr x0, =TCR_CONFIG_DEFAULT
    msr tcr_el1, x0

// set mair_el1 
    ldr x0, =( \
      (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
      (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) \
    )
    msr mair_el1, x0

// initialize identity page table
    adr x0, __PGD_start // PGD's page frame at 0x0
    adr x1, __PUD_start // PUD's page frame at 0x1000
// initialize PGD
    ldr x2, = BOOT_PGD_ATTR
    orr x2, x1, x2 // combine the physical address of next level page with attribute.
    str x2, [x0]
// initialize PUD
    ldr x2, = BOOT_PUD_ATTR
    mov x3, 0x00000000
    orr x3, x2, x3
    str x3, [x1] // 1st 1GB mapped by the 1st entry of PUD
    mov x3, 0x40000000
    orr x3, x2, x3
    str x3, [x1, 8] // 2nd 1GB mapped by the 2nd entry of PUD
// set ttbr0_el1 to PGU
    msr ttbr0_el1, x0 // load PGD to the bottom translation-based register (el0 mapping).
    msr ttbr1_el1, x0 // load PGD to the bottom translation-based register (el1 mapping).

    mrs x2, sctlr_el1
    orr x2 , x2, 1 // set first bit to 1 to enable MMU
    msr sctlr_el1, x2 // enable MMU, cache remains disabled

    ret


.section ".data"
_dtb: .dword 0


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
    ldr     x1, =__bss_start
    ldr     x2, =__bss_end
bss_zero:
    sub     x3, x2, x1
    cbz     x3, end 
    str     xzr, [x1], #8 // store zero to [x1] and add 8 to x1
    b bss_zero

end:
// from el2 to el1
    bl from_el2_to_el1

    adr     x1, _dtb
    ldr     x0, [x1]
// jump to kernel_main
    bl      kernel_main
    b       proc_hang
from_el2_to_el1:
    ldr x0, =__EL1_stack // Set el1 stack to __EL1_stack
    msr sp_el1, x0

    mov x0, (1 << 31) // Set hcr_el2.RW so EL1 uses aarch64
    msr hcr_el2, x0

    mov x0, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x0
    msr elr_el2, lr
    eret // return to EL1

.section ".data"
_dtb: .dword 0


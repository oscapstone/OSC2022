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
// stack 
    adr     x1, _start
    mov     sp, x1
// initialize .bss section
    ldr     x1, =__bss_start
    ldr     x2, =__bss_end
bss_zero:
    sub     x3, x2, x1
    cbz     x3, end 
    str     xzr, [x1], #8 // store zero to [x1] and add 8 to x1
    b bss_zero

end:
// jump to kernel_main
    bl      kernel_main
    b       proc_hang

.section ".text.boot"
.global _start
_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #0xff

    // cpu id > 0, stop
    cbz     x1, master_core 

proc_hang:  wfe
    b       proc_hang

// cpu id == 0
master_core:
    adr     x1, _start
    mov     sp, x1

    bl      kernel_main
    b       proc_hang

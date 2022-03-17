.section ".text"

_start:
    // let core with cpuid != 0 enter busy loop
    mrs x1, mpidr_el1
    and x1, x1, #3
    cbz x1, meminit
busyloop:
    wfe
    b busyloop
meminit:
    // clear the .bss segment to all zeros
    ldr x1, = __bss_start
    ldr x2, = __bss_end
    mov x3, #0
bssloop:
    cmp x1, x2
    bge bssdone
    str x3, [x1], #8
    b bssloop
bssdone:
    // set stack pointer and branch to main function.
    ldr x1, = _start
    mov sp, x1
    bl main
    b busyloop
.section ".text"
.global _start // decdie entry point

_start: 
    mov x20, x0
    mov x21, x1
    mov x22, x2
    mov x23, x3
    // only run program on core-0, others(1-3) are idle 
    mrs x1, mpidr_el1 // core identification, [7:0] - individual threads within a core
    and x1, x1, #3  // 3 => 0~3 (rpi3 has 4 cores)
    cbz x1, core0 // run progrm if core=0 else idle

idle:  
    wfe // wait for event (idle)
    b idle

core0: 
    // set the stack pointer at the address of code
    ldr x1, =_start // get address of code
    
    // set the uninitialized variable (bss section) to zero
    // x - 64bits, w - 32bits (4bytes is large enough for size)
    ldr x1, =_bss_start // _bss_start described in linker script
    ldr w2, =_bss_size // _bss_size described in linker script

init_loop: 
    cbz w2, relocat
    str xzr, [x1], #8 // set x1 to zero(xzr), and x1's address + 8bytes 
    sub w2, w2, #1
    b init_loop

relocat:
    // relocate kernel-1 from default address to link address
    ldr x1, =_load_start
    mov x0, x1 // store the load address as argument for receive function
    ldr x2, =_kernel1_start
    mov sp, x2 // set stack pointer 
    ldr w3, =_loader_size

relocat_loop:
    cbz w3, load_kernel - 4096
    ldr x4, [x1], #8
    str x4, [x2], #8
    sub w3, w3, #1
    b relocat_loop

load_kernel:
    bl uart_img_receiver // once relocate done, the receiver entry will be shift to link address
    mov x30, x0 // move kernel address to another register
    mov x0, x20 // move dtb address back to x0 as argument for main function
    mov x1, x21
    mov x2, x22
    mov x3, x23
    ret x30 // jump to return address of receiver

    
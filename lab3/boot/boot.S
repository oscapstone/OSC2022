.section ".text.boot"
.global _start
_start:
// x1 = current PC + offset to _dtb
    adr     x1, _dtb
    str     x0, [x1]

// stack 
    adr     x1, _start
    mov     sp, x1

// rebase boot loader to specific address 
// indicated by linker script
    bl rebase_bootloader

// slave cpu should not sleep before bootloader finishes rebase
// Since if they wake up, there next instruction will be invalid instruction
rebase_entry:
    // read cpu id, stop slave cores
    // cpu id == 0
    // store pointer to dtb to _dtb
    mrs     x1, mpidr_el1
    and     x1, x1, #0xff

    // cpu id > 0, stop
    cbz     x1, master_core 

proc_hang:  
	wfe
    b       proc_hang


master_core:
// stack 
    ldr     x1, =_start
    mov     sp, x1

// recover dtb to x0
    adr     x1, _dtb
    ldr     x0, [x1]


// jump to kernel_main
    bl      uart_recv_kernel
    
    b       proc_hang

// rebase our boot loader
rebase_bootloader:
    // manipulate our return address ( x30 a.k.a LR ) to the rebased bootloader
    // x19  =  current PC + offset to _start
    // x20  =  absolute address ( _start ) that specify by linker script  
    adr     x19, _start
    ldr     x20, =_start
    sub     x19, x19, x20
    sub     lr , lr, x19
    
    // x21   =  absolute address ( __bootloader_start ) that specify by linker script
    // x22   =  current PC + offset to __bootloader_start ( In linker script )
    ldr     x21, =__bootloader_start 
    adr     x22, __bootloader_start 
    ldr     x23, =__bootloader_size
    // start rebasing bootloader
copy_bootloader:
    cbz     x23, end_rebase_bootloader
    ldr     x24, [x22], #8
    str     x24, [x21], #8
    sub     x23, x23, #8
    b       copy_bootloader

// Since ldr and str memory operation should obey 8 bytes alignment rule, the code here would failed on real device 
//    cbz     x23, end_rebase_bootloader
//    ldr     x24, [x22], #1
//    str     x24, [x21], #1
//    sub     x23, x23, #1
//    b       copy_bootloader

    
    // return to rebased bootloader
end_rebase_bootloader:
    ret

.section ".data"
_dtb: .long 0

.section ".text"

_start:
    b 1

// set stack pointer and branch to main function.
1:
    ldr x0, = _stack_top
    mov sp, x0
    bl main

dead_loop:
    b dead_loop
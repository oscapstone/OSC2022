#include "malloc.h"
#include "exec.h"
#include "timer.h"

int exec(char* data){

    char* ustack = kmalloc(USTACK_SIZE);

    asm("msr elr_el1, %0\n\t"
        //"mov x1, 0x3c0\n\t"
        "msr spsr_el1, xzr\n\t"
        "msr sp_el0, %1\n\t"    // enable interrupt in EL0. You can do it by setting spsr_el1 to 0 before returning to EL0.
        "eret\n\t"
        :: "r" (data),
           "r" (ustack+USTACK_SIZE));

    kfree(ustack);
    return 0;
}
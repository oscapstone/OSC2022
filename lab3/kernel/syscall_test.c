#include "types.h"
#include "utils.h"
#include "debug/debug.h"
#include "asm.h"
uint64_t sys_hello(uint64_t x0){
    LOG("Enter sys_hello");
    printf("CurrentEL:  %x\r\n", get_currentEL());
    printf("DAIF:       %x\r\n", get_DAIF());
    printf("SPSR_EL1:   %p\r\n", (void*)get_SPSR_EL1());
    printf("ELR_EL1:    %p\r\n", (void*)get_ELR_EL1());
    printf("ESR_EL1:    %p\r\n", (void*)get_ESR_EL1());
    printf("SP     :    %p\r\n", (void*)get_SP());
    printf("\r\n");
    return x0;
}

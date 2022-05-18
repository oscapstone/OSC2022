#include "types.h"
uint32_t get_currentEL(){
    uint64_t curEL = 0;
    asm volatile("mrs %0, CurrentEL"
                 : "=&r" (curEL)
                 :
    );
    return curEL >> 2;
}
uint64_t get_SP_ELx(uint32_t x){
    uint64_t sp = 0;

    switch(x){
        case 0:
            asm volatile("mrs %0, SP_EL0"
                         : "=&r" (sp)
                         :
            );
            break;
        case 1:
            asm volatile("mrs %0, SP_EL1"
                         : "=&r" (sp)
                         :
            );
            break;
        case 2:
            asm volatile("mrs %0, SP_EL2"
                         : "=&r" (sp)
                         :
            );
            break;
        default:
            break;
    }


    return sp;
}
uint64_t get_DAIF(){
    uint64_t daif = 0;
    asm volatile("mrs %0, DAIF"
                 : "=&r" (daif)
                 :
    );
    return daif >> 6;
}

uint64_t get_SPSel(){
    uint64_t spsel = 0;
    asm volatile("mrs %0, spsel"
                 : "=&r" (spsel)
                 :
    );
    return spsel;
}
uint64_t get_ESR_EL1(){
    uint64_t esr_el1 = 0;
    asm volatile("mrs %0, esr_el1"
                 : "=&r" (esr_el1)
                 :
    );
    return esr_el1;
}
uint64_t get_SPSR_EL1(){
    uint64_t spsr_el1 = 0;
    asm volatile("mrs %0, spsr_el1"
                 : "=&r" (spsr_el1)
                 :
    );
    return spsr_el1;
}
uint64_t get_ELR_EL1(){
    uint64_t elr_el1 = 0;
    asm volatile("mrs %0, elr_el1"
                 : "=&r" (elr_el1)
                 :
    );
    return elr_el1;
}
uint64_t get_SP(){
    uint64_t sp = 0;
    asm volatile("mov %0, sp"
                 : "=&r" (sp)
                 :
    );
    return sp;
}

#ifndef MEM_ARMV8_H
#define MEM_ARMV8_H


#ifndef WITH_STDLIB
#include "type.h"
#else
#include <stdint.h>
#endif



void clean_dcache(volatile uint32_t va) {


    asm volatile("dc cvau, %0"
                :
                : "r" (va)
                :);
    //* clean the data cache with the mva(modified virtual address)
}







#endif
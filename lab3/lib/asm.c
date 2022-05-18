#include "types.h"
uint32_t get_currentEL(){
    uint64_t curEL = 0;
    asm volatile("mrs %0, CurrentEL"
                 : "=&r" (curEL)
                 :
    );
    return curEL >> 2;
}

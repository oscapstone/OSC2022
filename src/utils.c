#include "utils.h"

/* transfer big endian to little endian */
uint32_t big2little(uint32_t big){
    // use instruction
    uint32_t little = (0x000000ff & (big >> 24)) |
                      (0x0000ff00 & (big >>  8)) |
                      (0x00ff0000 & (big <<  8)) |
                      (0xff000000 & (big << 24));
    return little;    
}

unsigned long align_up(unsigned long n, unsigned long align)
{
    return (n + align - 1) & (~(align - 1));
}
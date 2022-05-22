#include "lib/bitops.h"

uint16_t ffs16(uint16_t val){
// val should not be zero! 
// The behavior is undefined when val is zer!
    uint16_t num = 0;
    if((val & 0xff) == 0){
        num += 8;
        val >>= 8;
    }
    if((val & 0xf) == 0){
        num += 4;
        val >>= 4;
    }
    if((val & 0x3) == 0){
        num += 2;
        val >>= 2;
    }
    if((val & 0x1) == 0)
        num += 1;

    return num;
}

uint64_t ffs64(uint64_t val){
// val should not be zero! 
// The behavior is undefined when val is zer!
    uint64_t num = 0;
    if((val & 0xffffffff) == 0){
        num += 32;
        val >>= 32;
    }
    if((val & 0xffff) == 0){
        num += 16;
        val >>= 16;
    }
    if((val & 0xff) == 0){
        num += 8;
        val >>= 8;
    }
    if((val & 0xf) == 0){
        num += 4;
        val >>= 4;
    }

    if((val & 0x3) == 0){
        num += 2;
        val >>= 2;
    }

    if((val & 0x1) == 0)
        num += 1;

    return num;
}

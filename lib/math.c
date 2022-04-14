#include "math.h"
#include "type.h"


uint64 log(uint64 x, uint64 base) {
    uint64 exp = 0;

    while(x) {
        exp++;
        x /= base;
    }

    if(exp > 0) {
        exp--;
    }
    
    return exp;
}

uint64 exp(uint64 base, uint64 pow) {
    uint64 y = 1;

    while(pow--) {
        y *= base;
    }

    return y;
}

uint64 log2(uint64 x) {
    return log(x, 2);
}

uint64 exp2(uint64 x) {
    return exp(2, x);
}
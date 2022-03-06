

#include "limits.h"
#ifndef WITH_STDLIB
#include "type.h"
#else
#include <stdint.h>
#endif
#include "ctype.h"
#include "strtol.h"



int64_t _strtoll(const char* str, char** endptr, int base) {


    /**
     * @brief
     * extract the leading
     * 
     */

    register int neg = 0;
    register int64_t cutoff;
    register int64_t cutlim;


    
    const char* s = str;
    int c;

    do {
        c = *s++;
    } while(c == ' ');


    if(c == '-') {
        neg = 1;
        c = *s++;
    } else if(c == '+')
        c = *s++;

    if( (base == 16 || base == 0) && 
        (c == '0' && (*s =='x' || *s == 'X'))) {
        base = 16;
        c = s[1];
        s += 2;
    }

    if(base == 0)
        base = c == '0' ? 8 : 10;

    cutoff = (neg) ? -LONG_MIN : LONG_MAX;
    cutlim = cutoff % base;
    cutoff /= base;


    int64_t acc, any;

    for(acc = 0, any = 0;; c = *s++) {

        if(is_digit(c)) {
            c = c - '0';
        } else if(is_alpha(c)) {
            c = is_upper(c) ? c - 'A' : c - 'a';
            c += 10;
        } else 
            break;

        if(c >= base)
            break;

        if(acc < 0 || acc >= cutoff || (acc == cutoff && c > cutlim)){
            any = -1;
        } else {
            any = 1;
            acc *= base;
            acc += c;
        }

    }

    if(any < 0) {
        return (neg) ? LONG_MIN : LONG_MAX;
    }

    if(neg) {
        acc = -acc;
    }

    if(endptr != 0)
        *endptr = (char*) (any ? s - 1 : str);

    return (acc);

}

uint64_t _strtoull(const char* str, char** endptr, int base) {

    return (uint64_t)_strtoll(str, endptr, base);
}

int32_t _strtol(const char* str, char** endptr, int base) {
    return (int32_t)_strtoll(str, endptr, base);
}

uint32_t _strtoul(const char* str, char** endptr, int base) {
    return (uint32_t)_strtoll(str, endptr, base);
}



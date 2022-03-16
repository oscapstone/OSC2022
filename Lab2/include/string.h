#ifndef _STRING_H
#define _STRING_H
#include "stdint.h"
int compare(char const *a, char const *b){
    //for(int i = 0; i<size; i++){
    //uart_puts(a);
    while(*a){
        if(*a != *b) return 0;
        if(*a == '\0' && *b == '\0') return 1; /// ????????
        a++; b++;
    }
    return 1; // 
}

char* itoa(int64_t val, int base){
    static char buf[32] = {0};
    int i = 30;
    if (val == 0) {
        buf[i] = '0';
        return &buf[i];
    }

    for (; val && i; --i, val /= base)
        buf[i] = "0123456789abcdef"[val % base];

    return &buf[i + 1];
}
#endif
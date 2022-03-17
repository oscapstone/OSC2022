#include "string.h"
#include "memory.h"
#include "uart.h"

void delay_cycle(unsigned int n) {
    while(n--) { 
        asm volatile("nop"); 
    } 
}

void delay_ms(unsigned int n) {
    register unsigned long f, t, r;
    // get the current counter frequency
    asm volatile ("mrs %0, cntfrq_el0" : "=r"(f));
    // read the current counter
    asm volatile ("mrs %0, cntpct_el0" : "=r"(t));
    
    t += ((f / 1000) * n) / 1000;
    do {
        asm volatile ("mrs %0, cntpct_el0" : "=r"(r));
    } while(r < t);
}

unsigned int str2num(char* str, int len) {
    int num = 0;
    char c;

    while(len--) {
        c = *(str++);
        num = num * 10 + c - '0';
    } 

    return num;
}
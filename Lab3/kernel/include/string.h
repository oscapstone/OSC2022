#ifndef _STRING_H
#define _STRING_H
#include "stdint.h"

int strcmp(const char *p1, const char *p2);
char *strstr(const char *s, const char *find);

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

int strcmp(const char *p1, const char *p2) {
  const unsigned char *s1 = (const unsigned char *)p1;
  const unsigned char *s2 = (const unsigned char *)p2;
  unsigned char c1, c2;
  do {
    c1 = (unsigned char)*s1++;
    c2 = (unsigned char)*s2++;
    if (c1 == '\0') return c1 - c2;
  } while (c1 == c2);
  return c1 - c2;
}
#endif
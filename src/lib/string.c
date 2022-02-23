#include "string.h"

void *memcpy (void *dst, void *src, size_t n) {

    unsigned char *d = (unsigned char *)dst;
    unsigned char *s = (unsigned char *)src;

    for (int i = 0; i < n; i++) {
        *d = *s;
        d++; s++;
    }

    return dst;
}

void *memset (void *dst, int value, size_t n) {

    unsigned char *d = (unsigned char *)dst;

    for (int i = 0; i < n; i++) {
        *d = (unsigned char) value;
        d++;
    }

    return dst;
}

long strlen (char *s) {

    long n = 0;
    
    while (*s) {
        s++; n++;
    } 

    return n;
}

char *strcpy (char *dst, char *src) {

    char *d = dst;

    while (*src) {
        *d = *src;
        d++; src++;
    }

    *d = '\0';

    return dst;
}

char *strncpy (char *dst, char *src , size_t n) {

    char *d = dst;

    while (n > 0 && *src) {
        *d = *src;
        d++; src++; n--;
    }

    while (n > 0) {
        *d = '\0';
        d++; n--;
    }

    return dst;
}

char *strcat (char *dst, char *src) {

    char *d = dst;
    
    while (*d) d++;

    while (*src) {
        *d = *src; 
        d++; src++;
    }

    *d = '\0';

    return dst;
}

char *strncat (char *dst, char *src, size_t n) {
    
    char *d = dst;
    
    while (*d) d++;

    while (n > 0 && *src) {
        *d = *src; 
        d++; src++; n--;
    }

    *d = '\0';

    return dst;
}


int strcmp (char *s1, char*s2) {

    int v = 0;

    while (*s1 != '\0' && *s2 != '\0') {
        
        if (*s1 < *s2) 
        {
            v = -1;
            break;
        } 
        else if (*s1 > *s2) 
        {
            v = 1;
            break;
        }

        s1++; s2++;
    }

    return v;
}

int strncmp (char *s1, char *s2, size_t n) {

    int v = 0;

    while (n > 0 && *s1 != '\0' && *s2 != '\0') {
        
        if (*s1 < *s2) 
        {
            v = -1;
            break;
        } 
        else if (*s1 > *s2) 
        {
            v = 1;
            break;
        }

        n--; s1++; s2++;
    }

    return v;
}
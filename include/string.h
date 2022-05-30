#include "mini_uart.h"
static inline int strcmp (const char *p1, const char *p2)
{
    const unsigned char *s1 = (const unsigned char *) p1;
    const unsigned char *s2 = (const unsigned char *) p2;
    unsigned char c1, c2;
    do
        {
        c1 = (unsigned char) *s1++;
        c2 = (unsigned char) *s2++;
        if (c1 == '\0')
            return c1 - c2;
        }
    while (c1 == c2);

    return c1 - c2;
}
int strncmp(const char *s1, const char *s2, unsigned int n);
int strlen(char *s);
unsigned int str2int(char *s);
static inline char* strcpy(char* destination, const char* source)
{
    // return if no memory is allocated to the destination
    if (destination == nullptr) {
        return (char*)nullptr;
    }
 
    // take a pointer pointing to the beginning of the destination string
    char *ptr = destination;
 
    // copy the C-string pointed by source into the array
    // pointed by destination
    while (*source != '\0')
    {
        *destination = *source;
        destination++;
        source++;
    }
 
    // include the terminating null character
    *destination = '\0';
 
    // the destination is returned by standard `strcpy()`
    return ptr;
}
static inline char* strncpy(char* destination, const char* source,int n)
{
    // return if no memory is allocated to the destination
    if (destination == nullptr) {
        return (char*)nullptr;
    }
 
    // take a pointer pointing to the beginning of the destination string
    char *ptr = destination;
 
    // copy the C-string pointed by source into the array
    // pointed by destination
    int i=0;
    while (*source != '\0' && i<n)
    {
        *destination = *source;
        destination++;
        source++;
        i++;
    }
 
    // include the terminating null character
    *destination = '\0';
 
    // the destination is returned by standard `strcpy()`
    return ptr;
}


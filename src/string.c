#include "string.h"

void* memcpy(void* dest, const void* src, unsigned long count)
{
    char *d = (char *) dest;
    char *s = (char *) src;

    for (int i = 0; i < count; i++)
        *(d++) = *(s++);

    return dest;
}

int strncmp( const char *lhs, const char *rhs, unsigned long count )
{
    int value;
    char *s1 = lhs;
    char *s2 = rhs;

    s1--, s2--;
    do {
        s1++, s2++;
        if (*s1 == *s2) {
            value = 0;
        }
        else if (*s1 < *s2) {
            value = -1;
            break;
        }
        else {
            value = 1;
            break;
        }
    } while (--count && *s1 != 0 && *s2 != 0);

    return value;
}

int strcmp(char *s1, char *s2)
{
    int value;
 
    s1--, s2--;

    do {
        s1++, s2++;

        if (*s1 == *s2) {
            value = 0;
        }
        else if (*s1 < *s2) {
            value = -1;
            break;
        }
        else {
            value = 1;
            break;
        }
    } while (*s1 != 0 && *s2 != 0);

    return value;
}

char *strcat( char *dest, const char *src )
{
    char *tmp = dest;

    while (*tmp) tmp++;
    while (*src) *(tmp++) = *(src++);
    *tmp = 0;

    return dest;
}

char* strtok( char* str, const char delim )
{
    char *start;
    int tok_has_data = 0;

    if (str != NULL) start = str, ptr = str;
    else start = ptr;

    while (*ptr) {
        if (*ptr == delim) {

            if (tok_has_data) {
                *(ptr++) = '\0';
                return start;
            }
            else {
                start++, ptr++;
            }
        }
        else {
            
            tok_has_data = 1;
            ptr++;
        }
    }
    
    if (tok_has_data == 1) return start;
    else return NULL;
}

size_t strlen( const char* str )
{
    const char* end = str;

    for( ; *end != '\0'; ++end) ;

    return end - str;
}
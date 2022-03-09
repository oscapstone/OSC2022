#include "utils.h"
#include "mini_uart.h"

int compare_string(const char *s1, const char *s2) {
    unsigned char c1, c2;

    do {
        c1 = (unsigned char)*s1++;
        c2 = (unsigned char)*s2++;
        if (c1 == '\0') {
            return c1 - c2;
        }
    } while (c1 == c2);

    return c1 - c2;
}

void uintoa(char *out, unsigned int i)
{   
    unsigned int index = 0;
    char tmp[20];
    do {
        unsigned char m = i % 16;
        i >>= 4;

        char c = m + '0';
        if (m == 10)
            c = 'A';
        else if (m == 11)
            c = 'B';
        else if (m == 12)
            c = 'C';
        else if (m == 13)
            c = 'D';
        else if (m == 14)
            c = 'E';
        else if (m == 15)
            c = 'F';

        tmp[index++] = c;

    } while (i);
    
    index--;
    for (unsigned int i = 0; i <= index; ++i)
        out[i] = tmp[index - i];
    out[index + 1] = '\0';
}

unsigned int getIntegerFromString(const char *str) {
    unsigned int value = 0u;

    while (*str) {
        if(*str >= '0' && *str<= '9'){
            value = value * 10u + (*str - '0');
        }
        ++str;
    }
    return value;
}

unsigned long getHexFromString(const char *str) {
    unsigned long value = 0u;

    while (*str) {
        if(*str >= '0' && *str <= '9'){
            value = value*16 + *str - '0';
        }else if(*str >= 'a' && *str <= 'z'){
            value = value*16 + *str - 'a' + 10u;
        }else if(*str >= 'A' && *str <= 'Z'){
            value = value*16 + *str - 'A' + 10u;
        }
        ++str;
    }
    return value;
}


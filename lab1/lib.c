#include "uart.h"

int strcmp(char *s1, char *s2) {
    
    while ( *s1 != '\0' && *s2 != '\0' ) {
        // uart_send(*s1);
        // uart_send('.');
        // uart_send(*s2);
        // uart_send('.');

        if ( *s1 != *s2 ) {
            return (int) (*s1 - *s2);
        }
        s1++;
        s2++;
    }
    // uart_puts("n\n");
    if ( *s1 == *s2 ) {
        return 0;
    } 

    return (int) (*s1 - *s2);

}

int len(char *s) {
    int digits = 0;
    while(*s++) {
        digits++;
    }

    return digits;
}

int atoi(char *s) {
    char *str;
    int digits, num = 0, tmp;
    
    str = s;
    digits = len(str);
    
    for(int i=0; i<digits; i++) {
        tmp = *str++ - '0';
        for (int j = 0; j<digits-i-1; j++) {
            tmp *= 10;
        }
        num += tmp;
    }
    return num;
}

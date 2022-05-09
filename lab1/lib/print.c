#include "lib/print.h"
#include "lib/string.h"
#include "types.h"
#include "peripherals/mini_uart.h"
#include <stdarg.h>

int32_t printf(char *fmt, ...){
    char str[64];
    char* p;
    volatile int32_t count = 0;
    va_list ap;
    
    va_start(ap, fmt);

    while(*fmt){
        char c = *fmt++; 
        if(c == '%'){
            c = *fmt++;
            switch(c){
                case 'u':
                    uint32_t uval = va_arg(ap, uint32_t);
                    utoa(uval, str, 10);
                    p = str;
                    while(*p){
                        mini_uart_write(*p++);
                        count++;
                    }
                    break;
                case 'd':
                    int val = va_arg(ap, int);
                    itoa(val, str, 10);
                    p = str;
                    while(*p){
                        mini_uart_write(*p++);
                        count++;
                    }
                    break;
                case 'l':
                    int64_t lval = va_arg(ap, int64_t);
                    ltoa(lval, str, 10);
                    p = str;
                    while(*p){
                        mini_uart_write(*p++);
                        count++;
                    }
                    break;
                case 'c':
                    char ch = va_arg(ap, int);
                    mini_uart_write(ch);
                    count++;
                    break;
                case 'p':
                    void* addr = va_arg(ap, void*);
                    utoa((uint64_t)addr, str, 16);

                    mini_uart_write('0');
                    mini_uart_write('x');
                    count += 2;
                    
                    p = str;
                    while(*p){
                        mini_uart_write(*p++);
                        count++;
                    }
                    break;

                    break;
                case 'x':
                    uint32_t hex = va_arg(ap, uint32_t);
                    utoa(hex, str, 16);
                    p = str;
                    while(*p){
                        mini_uart_write(*p++);
                        count++;
                    }

                    break;
                case 's':
                    char* s = va_arg(ap, char*);
                    p = s;
                    while(*p){
                        mini_uart_write(*p++);
                        count++;
                    }
                    break;
                default:
                case '%':
                    mini_uart_write('%');
                    count++;
            }
        }else{
            mini_uart_write(c);
            count++;
        }
    }
    va_end(ap);
    return count;
}

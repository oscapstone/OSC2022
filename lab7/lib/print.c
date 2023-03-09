#include "lib/print.h"

int32_t aio_print(char* fmt, ...){

}
int32_t printf(char *fmt, ...){
    uint32_t uval;
    int32_t val;
    int64_t lval;
    char str[64], ch;
    char* p, *s;
    void* addr; 
    volatile int32_t count = 0;
    va_list ap;
    
    va_start(ap, fmt);

    while(*fmt){
        char c = *fmt++; 
        if(c == '%'){
            c = *fmt++;
            switch(c){
                case 'u':
                    uval = va_arg(ap, uint32_t);
                    utoa(uval, str, 10);
                    p = str;
                    while(*p){
                        mini_uart_write(*p++);
                        count++;
                    }
                    break;
                case 'd':
                    val = va_arg(ap, int);
                    itoa(val, str, 10);
                    p = str;
                    while(*p){
                        mini_uart_write(*p++);
                        count++;
                    }
                    break;
                case 'l':
                    lval = va_arg(ap, int64_t);
                    ltoa(lval, str, 10);
                    p = str;
                    while(*p){
                        mini_uart_write(*p++);
                        count++;
                    }
                    break;
                case 'c':
                    ch = va_arg(ap, int);
                    mini_uart_write(ch);
                    count++;
                    break;
                case 'p':
                    addr = va_arg(ap, void*);
                    ultoa((uint64_t)addr, str, 16);

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
                    uval = va_arg(ap, uint32_t);
                    ultoa(uval, str, 16);
                    p = str;
                    while(*p){
                        mini_uart_write(*p++);
                        count++;
                    }

                    break;
                case 's':
                    s = va_arg(ap, char*);
                    p = s;
                    while(*p){
                        mini_uart_write(*p++);
                        count++;
                    }
                    break;
                case '%':
                    mini_uart_write('%');
                    count++;
                default:
                    mini_uart_write('%');
                    mini_uart_write(c);
                    count += 2;
            }
        }else{
            mini_uart_write(c);
            count++;
        }
    }
    va_end(ap);
    return count;
}
int32_t putchar(uint8_t ch){
    mini_uart_write(ch);
    return ch;
}
int32_t getchar(){
    return mini_uart_aio_read();
}

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
                        while(mini_uart_aio_write(*p++) != 1);
                        count++;
                    }
                    break;
                case 'd':
                    val = va_arg(ap, int);
                    itoa(val, str, 10);
                    p = str;
                    while(*p){
                        while(mini_uart_aio_write(*p++) != 1);
                        count++;
                    }
                    break;
                case 'l':
                    lval = va_arg(ap, int64_t);
                    ltoa(lval, str, 10);
                    p = str;
                    while(*p){
                        while(mini_uart_aio_write(*p++) != 1);
                        count++;
                    }
                    break;
                case 'c':
                    ch = va_arg(ap, int);
                    while(mini_uart_aio_write(ch) != 1);
                    count++;
                    break;
                case 'p':
                    addr = va_arg(ap, void*);
                    utoa((uint64_t)addr, str, 16);

                    while(mini_uart_aio_write('0') != 1);
                    while(mini_uart_aio_write('x') != 1);
                    count += 2;
                    
                    p = str;
                    while(*p){
                        while(mini_uart_aio_write(*p++) != 1);
                        count++;
                    }
                    break;

                    break;
                case 'x':
                    uval = va_arg(ap, uint32_t);
                    utoa(uval, str, 16);
                    p = str;
                    while(*p){
                        while(mini_uart_aio_write(*p++) != 1);
                        count++;
                    }

                    break;
                case 's':
                    s = va_arg(ap, char*);
                    p = s;
                    while(*p){
                        while(mini_uart_aio_write(*p++) != 1);
                        count++;
                    }
                    break;
                case '%':
                    while(mini_uart_aio_write('%') != 1);
                    count++;
                default:
                    while(mini_uart_aio_write('%') != 1);
                    while(mini_uart_aio_write(c) != 1);
                    count += 2;
            }
        }else{
            while(mini_uart_aio_write(c) != 1);
            count++;
        }
    }
    va_end(ap);
    return count;
}
int32_t putchar(uint8_t ch){
    while(mini_uart_aio_write(ch) != 1);
    return ch;
}
int32_t getchar(){
    return mini_uart_aio_read();
}

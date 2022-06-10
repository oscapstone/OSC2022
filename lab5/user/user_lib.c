#include "user_lib.h"

char int_table[] = "0123456789";
char hex_table[] = "0123456789abcdef";

uint64_t getpid(void){
    asm volatile("mov x8, 0\n\t"
                 "svc 0\n\t"
                 :
                 :
    );
}

size_t uart_read(char *buf, size_t size){
    asm volatile("mov x0, %[buf]\n\t"
                 "mov x1, %[size]\n\t"
                 "mov x8, 1\n\t"
                 "svc 0\n\t"
                 :
                 :[buf] "r" (buf), [size] "r" (size)
    );
}

size_t uart_write(char *buf, size_t size){
    asm volatile("mov x0, %[buf]\n\t"
                 "mov x1, %[size]\n\t"
                 "mov x8, 2\n\t"
                 "svc 0\n\t"
                 :
                 :[buf] "r" (buf), [size] "r" (size)
    );
}

uint64_t fork(void){
    asm volatile("mov x8, 4\n\t"
                 "svc 0\n\t"
                 :
                 :
    );
}

void debug_info(void){
    asm volatile("mov x8, 5\n\t"
                 "svc 0\n\t"
                 :
                 :
    );
}

void delay(uint64_t clock){
    for(uint64_t i = 0 ; i < clock ; i++) asm volatile("nop");
}

size_t strlen(const char* s){
    size_t i = 0;
    while(s[i++]);
    i--;
    return i;
}


char * itoa(int32_t value, char* str, uint32_t base){
    char buf[32];
    uint32_t val;
    volatile int i = 0, j = 0;

    switch(base){
        case 16:
            val = (uint32_t)value;
            do{
                buf[i] = hex_table[val & 15];
                i++;
            }while(val >>= 4);
            buf[i] = '\0';
            break;
        default:
        case 10:
            if(value < 0){
                val = -value;
                do{
                    buf[i] = int_table[val % 10];
                    i++;
                }while(val /= 10);
                buf[i] = '-';
                buf[++i] = '\0';
            }else{
                val = value;
                do{
                    buf[i] = int_table[val % 10];
                    i++;
                }while(val /= 10);
                buf[i] = '\0';
            }
    }
    i--;
    do{
        str[j] = buf[i];
        j++;i--;
    }while(i >= 0);
    str[j] = '\0';
    return str;
}

char * utoa(uint32_t value, char* str, uint32_t base){
    char buf[32];
    volatile int i = 0, j = 0;
    uint32_t val = value;

    switch(base){
        case 16:
            do{
                buf[i] = hex_table[val & 15];
                i++;
            }while(val >>= 4);
            buf[i] = '\0';
            break;
        default:
        case 10:
            do{
                buf[i] = int_table[val % 10];
                i++;
            }while(val /= 10);
            buf[i] = '\0';
    }
    i--;
    do{
        str[j] = buf[i];
        j++;i--;
    }while(i >= 0);
    str[j] = '\0';
    return str;
}
char * ltoa(int64_t value, char* str, uint32_t base){
    char buf[32];
    volatile int i = 0, j = 0;
    uint64_t val;

    switch(base){
        case 16:
            val = (uint64_t)value;
            do{
                buf[i] = hex_table[val & 15];
                i++;
            }while(val >>= 4);
            buf[i] = '\0';
            break;
        default:
        case 10:
            if(value < 0){
                val = -value;
                do{
                    buf[i] = int_table[val % 10];
                    i++;
                }while(val /= 10);
                buf[i] = '-';
                buf[++i] = '\0';
            }else{
                val = value;
                do{
                    buf[i] = int_table[val % 10];
                    i++;
                }while(val /= 10);
                buf[i] = '\0';
            }
    }
    i--;
    do{
        str[j] = buf[i];
        j++;i--;
    }while(i >= 0);
    str[j] = '\0';
    return str;

}
char * ultoa(uint64_t value, char* str, uint32_t base){
    char buf[32];
    volatile int i = 0, j = 0;
    uint64_t val = value;

    switch(base){
        case 16:
            do{
                buf[i] = hex_table[val & 15];
                i++;
            }while(val >>= 4);
            buf[i] = '\0';
            break;
        default:
        case 10:
            do{
                buf[i] = int_table[val % 10];
                i++;
            }while(val /= 10);
            buf[i] = '\0';
    }
    i--;
    do{
        str[j] = buf[i];
        j++;i--;
    }while(i >= 0);
    str[j] = '\0';
    return str;

}

uint64_t atoul(const char* str){
    uint64_t val = 0;
    const char* ps = str;
    while(*ps){
        val *= 10;
        val += (*ps - '0');
        ps++;
    }
    return val;
}

int32_t printf(char *fmt, ...){
    uint32_t uval;
    int32_t val;
    int64_t lval;
    char str[64], ch;
    char* s;
    void* addr;
    size_t tmp_c;
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
                    tmp_c = strlen(str);
                    uart_write(str, tmp_c);
                    count += tmp_c;
                    break;
                case 'd':
                    val = va_arg(ap, int);
                    itoa(val, str, 10);
                    uart_write(str, strlen(str));
                    break;
                case 'l':
                    lval = va_arg(ap, int64_t);
                    ltoa(lval, str, 10);
                    tmp_c = strlen(str);
                    uart_write(str, tmp_c);
                    count += tmp_c;
                    break;
                case 'c':
                    ch = va_arg(ap, int);
                    uart_write(&ch, 1);
                    count++;
                    break;
                case 'p':
                    addr = va_arg(ap, void*);
                    utoa((uint64_t)addr, str, 16);

                    uart_write("0x", 2);
                    count += 2;

                    tmp_c = strlen(str);
                    uart_write(str, tmp_c);
                    count += tmp_c;
                    
                    break;

                    break;
                case 'x':
                    uval = va_arg(ap, uint32_t);
                    utoa(uval, str, 16);
                    tmp_c = strlen(str);
                    uart_write(str, tmp_c);
                    count += tmp_c;

                    break;
                case 's':
                    s = va_arg(ap, char*);
                    tmp_c = strlen(s);
                    uart_write(s, tmp_c);
                    count += tmp_c;
                    break;
                case '%':
                    uart_write("%", 1);
                    count++;
                default:
                    uart_write("%", 1);
                    uart_write(&c, 1);
                    count += 2;
            }
        }else{
            uart_write(&c, 1);
            count++;
        }
    }
    va_end(ap);
    return count;
}


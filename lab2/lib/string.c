#include "lib/string.h"
#include "types.h"
char *int_table = "0123456789";
char *hex_table = "0123456789abcdef";
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
int32_t strcmp(char* str1, char* str2){
    do{
        if(*str1 > *str2) return 1;
        else if(*str1 < *str2) return -1;
        str1++;str2++;
    }while(*str1 != '\0');
    return 0;
}
void* memcpy(void* dst, const void* src, size_t n){
    const uint64_t* ps = src;
    uint64_t*pd = dst;
    size_t i, q = n / 8, r = n % 8;
    for(i = 0 ; i < q ; i++) pd[i] = ps[i];
    for(i = 0 ; i < r ; i++) ((uint8_t*)pd)[i] = ((uint8_t*)ps)[i];

    return dst;
}
void* memset(void* s, int c, size_t n){
    uint8_t* ps = s;
    for(size_t i = 0 ; i < n ; i++) ps[i] = c;
    return s;
}

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
    }while(*str1 != '\0' || *str2 != '\0');
    return 0;
}
size_t strlen(const char* s){
    size_t i = 0;
    while(s[i++]);
    i--;    
    return i;
}
void* memcpy(void* dst, const void* src, size_t n){
    const uint8_t* ps = src;
    uint8_t *pd = dst;
    size_t i;
    for(i = 0 ; i < n ; i++) pd[i] = ps[i];
    return dst;
}
char *strcpy(char *dest, const char *src){
    memcpy(dest, src, strlen(src) + 1);
}
void* memset(void* s, int c, size_t n){
    uint8_t* ps = s;
    for(size_t i = 0 ; i < n ; i++) ps[i] = c;
    return s;
}

int32_t memcmp(void* m1, const void* m2, size_t n){
    size_t i = 0;
    char *s1 = m1;
    const char* s2 = m2;
    while(i < n){
        if(s1[i] > s2[i]) return 1;
        else if(s1[i] < s2[i]) return -1;
        i++;
    }
    return 0;
}

uint8_t hex2dec(char hex){
    if('0' <= hex && hex <= '9'){
        return (uint8_t)hex - (uint8_t)'0';
    }else if('a' <= hex && hex <= 'f'){
        return (uint8_t)hex - (uint8_t)'a' + 10;
    }else{
        return (uint8_t)hex - (uint8_t)'A' + 10;
    }
}

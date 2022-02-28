#include <stdint.h>
#include <stddef.h>

int strcmp(const char* a, const char* b)
{
    int i=0;
    for(; a[i] && b[i]; i++){
        if(a[i]<b[i]) return -1;
        else if(a[i]>b[i]) return 1;
    }
    if(a[i]) return 1;
    else if(b[i]) return -1;
    return 0;
}

int strncmp(const char* a, const char* b, int n)
{
    int i=0;
    for(; a[i] && b[i] && i<n; i++){
        if(a[i]<b[i]) return -1;
        else if(a[i]>b[i]) return 1;
    }
    if(i==n) return 0;
    if(a[i]) return 1;
    else if(b[i]) return -1;
    return 0;
}

char u42hex(uint32_t num)
{
    if(num<=9) return '0'+num;
    if(num>9 && num<16) return 'a'+num-10;
    return '?';
}

void u322hex(uint32_t num, char* buf, size_t len)
{
    len--;
    buf[0] = '0';
    buf[1] = 0;
    size_t i = 0;
    while(num && i<len){
        buf[i++] = u42hex(num&0xf);
        num = num>>4;
    }
    for(int j=0;j<i/2;j++){
        char tmp = buf[j];
        buf[j] = buf[i-j-1];
        buf[i-j-1] = tmp;
    }
    buf[(i>0?i:1)] = 0;
}

void u642hex(uint64_t num, char* buf, size_t len)
{
    len--;
    buf[0] = '0';
    buf[1] = 0;
    size_t i = 0;
    while(num && i<len){
        buf[i++] = u42hex(num&0xf);
        num = num>>4;
    }
    for(int j=0;j<i/2;j++){
        char tmp = buf[j];
        buf[j] = buf[i-j-1];
        buf[i-j-1] = tmp;
    }
    buf[(i>0?i:1)] = 0;
}
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <uart.h>
#include <kmalloc.h>

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

void *memcpy(void *dst, const void *src, size_t n)
{
    size_t npage = n / sizeof(uint32_t);
    size_t noff = n % sizeof(uint32_t);
    for(int i=0;i<npage;i++){
        //uart_putshex(i);
        *((uint32_t*)dst + i) = *((uint32_t*)src + i);
    }
    for(int i=0;i<noff;i++){
        //uart_putshex(i);
        *((char *)((uint32_t*)dst + npage) + i) = *((char *)((uint32_t*)src + npage) + i);
    }
    return dst;
}

size_t strlen(const char *str)
{
    int l = 0;
    while(*str){
        l++;
        str++;
    }
    return l;
}

char u42hex(uint32_t num)
{
    if(num<=9) return '0'+num;
    if(num>9 && num<16) return 'a'+num-10;
    return '?';
}

size_t u322hex(uint32_t num, char* buf, size_t len)
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
    return i>0?i:1;
}

size_t u642hex(uint64_t num, char* buf, size_t len)
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
    return i>0?i:1;
}

int atoi(const char* buf)
{
    int num = 0;
    int i=0;
    while(buf[i]){
        num *= 10;
        num += buf[i]-'0';
        i++;
    }
    return num;
}

size_t u642dec(uint64_t num, char* buf, size_t len)
{
    len--;
    buf[0] = '0';
    buf[1] = 0;
    size_t i = 0;
    while(num && i<len){
        buf[i++] = '0'+(num%10);
        num = num/10;
    }
    for(int j=0;j<i/2;j++){
        char tmp = buf[j];
        buf[j] = buf[i-j-1];
        buf[i-j-1] = tmp;
    }
    buf[(i>0?i:1)] = 0;
    return i>0?i:1;
}

size_t strncpy(char *dst, char *src, size_t len)
{
    int i=0;
    for(;src[i] && i<len-1;i++){
        dst[i] = src[i];
    }
    dst[i] = 0;
    return i;
}

//#define format_buf_len 0x800
//char format_buf_global[format_buf_len];
//char *format_buf;
//uint64_t format_buf_len;

char *vstrformat(char *buf, uint64_t len, const char *format, va_list ap)
{
    int i = 0, oi = 0;
    //va_list ap;
    //va_start(ap, format);
    for(;format[i] && oi < len-1;i++){
        if(format[i] != '%'){
            buf[oi++] = format[i];
        }
        else{
            i++;
            if(!format[i]){
                break;
            }
            switch (format[i]){
                case '%':
                    buf[oi++] = '%';
                    break;
                case 'd':
                    oi += u642dec(va_arg(ap, uint64_t), &buf[oi], len-oi-1);
                    //oi++;
                    break;
                case 'x':
                    oi += u642hex(va_arg(ap, uint64_t), &buf[oi], len-oi-1);
                    //oi++;
                    break;
                case 's':
                    oi += strncpy(&buf[oi], va_arg(ap, char *), len-oi-1);
                    break;
            }
        }
    }
    buf[oi] = 0;
    //va_end(ap);
    return buf;
}

char *strformat(char *buf, uint64_t len, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vstrformat(buf, len, format, ap);
    va_end(ap);
    return buf;
}
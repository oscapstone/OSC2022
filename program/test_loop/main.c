#include <stdint.h>
#include <stddef.h>

int getpid()
{
    asm(
        "mov x8, #0\t\n"
        "svc 0"
    );
    int ret;
    asm("mov %0, x0":"=r"(ret));
    return ret;
}

size_t uartread(const char buf[], size_t size)
{
    asm(
        "mov x8, #1\t\n"
        "svc 0"
    );
    size_t ret;
    asm("mov %0, x0":"=r"(ret));
    return ret;
}

size_t uartwrite(const char buf[], size_t size)
{
    asm(
        "mov x8, #2\t\n"
        "svc 0"
    );
    size_t ret;
    asm("mov %0, x0":"=r"(ret));
    return ret;
}

int exec(const char *name, char *const argv[])
{
    asm(
        "mov x8, #3\t\n"
        "svc 0"
    );
    size_t ret;
    asm("mov %0, x0":"=r"(ret));
    return ret;
}

void exit(int status)
{
    asm(
        "mov x8, #5\t\n"
        "svc 0"
    );
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

void uart_print(const char* buf)
{
    int i=0;
    while(buf[i]){
        uartwrite(&buf[i], 1);
        i++;
    }
}

void uart_puts(const char* buf)
{
    uart_print(buf);
    uartwrite("\n\r", 2);
}

size_t uart_gets(char* buf)
{
    int i=0;
    while(uartread(&buf[i], 1)){
        if(buf[i]=='\n'){
            buf[i] = 0;
            break;
        }
        i++;
    }
    return i;
}


void uart_print_hex(uint64_t num)
{
    char buf[0x10];
    u642hex(num, buf, 0x10);
    uart_print(buf);
}

void uart_putshex(uint64_t num)
{
    uart_print_hex(num);
    uart_puts("");
}

void init()
{
    //uartwrite("User process test!!", 19);
    uart_puts("User process test!! Loop...");
    int pid = getpid();
    while(1){
        uart_print("Loop Pid: 0x");
        uart_putshex(pid);
        for(int i=0;i<10000000;i++);
    }
}
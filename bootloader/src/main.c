#include <uart.h>
#include <shell.h>
#include <read.h>
#include <string.h>

/* bootloader entry point */
extern unsigned long _bootstart;

/* kernel entry point */
extern unsigned long _start;

/* kernel image size */
extern unsigned long __kernel_image_size;

int begin = 1;

void self_relocation(){
    char *src = (char *)&_start;
    char *dst = (char *)0x60000;
    unsigned long size = (unsigned long)&__kernel_image_size;
    for(unsigned long i = 0; i < size; i++){
        dst[i] = src[i];
    }
    //Cast the address back to a function and call it.
    void (*func_ptr)() = (void (*)())&_bootstart;
    func_ptr();

    // asm volatile(
    //     "ldr    x1, 0x60c48;"
    //     "br     x1;"
    // );
}

int main(){
    if(begin){
        begin = 0;
        self_relocation();
        uart_puts("2\n");
    }
    uart_init();

    char buf[15];
    char *kernel = (char *)0x80000;

    memset(buf, '\0', 15);
    readline(buf, 15);
    int size = atoi(buf);
    uart_puts("kernel image size: ");
    uart_puts(buf);
    uart_puts("\n");

    while(size--){
        char c = uart_getc();
        *kernel++ = c;
    }

    uart_puts("kernel image loaded\n");

    // void (*kernel_ptr)() = (void (*)())kernel;
    // kernel_ptr();
    asm volatile(
        "ldr    x1, 0x80000;"
        "br     x1;"
    );
 
    
    return 0;
}
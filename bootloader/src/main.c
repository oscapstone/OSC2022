#include <uart.h>
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
}

int main(){
    if(begin){
        begin = 0;
        self_relocation();
    }
    uart_init();

    char buf[15];
    char *kernel = (char *)0x80000;

    memset(buf, '\0', 15);
    readline(buf, 15);
    uart_puts("kernel image size: ");
    int size = atoi(buf);
    uart_puts(buf);
    uart_puts("\nloading...\n");

    while(size--){
        *kernel++ = uart_getc();
    }


    void (*kernel_ptr)() = (void *)0x80000;
    kernel_ptr();
    // asm volatile(
    //     "mov    x0, x10;"
    //     "mov    x1, x11;"
    //     "mov    x2, x12;"
    //     "mov    x3, x13;"
    //     "mov    x15, 0x80000;"
    //     "br     x15;"
    // );
 
    
    return 0;
}
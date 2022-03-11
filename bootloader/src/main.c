#include <uart.h>
#include <shell.h>
#include <read.h>
#include <string.h>

int main(){
    uart_init();
    char buf[15];
    char *kernal = (char *)0x80000;

    memset(buf, '\0', 15);
    readnbyte(buf, 10);
    int size = atoi(buf);
    uart_puts("kernal image size: ");
    uart_puts(buf);
    uart_puts("\n");

    while(size--){
        char c = uart_getc();
        *kernal++ = c;

        itoa(size,buf);
        uart_puts(buf);
        uart_puts("\n");
    }

    uart_puts("kernal image loaded\n");

    // Cast the address back to a function and call it.
    void (*call_function)() = (void (*)())kernal;
    call_function();
    
    return 0;
}
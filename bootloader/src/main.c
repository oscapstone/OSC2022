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
    unsigned int size = atoi(buf);
    uart_puts("kernal image size: ");
    uart_puts(buf);
    uart_puts("\n");


    for(int i = 0; i < size; i++){
        *kernal++ = uart_getc();
    }

    uart_puts("kernal image loaded\n");

    // Cast the address back to a function and call it.
    void (*call_function)() = (void (*)())kernal;
    call_function();
    
    return 0;
}
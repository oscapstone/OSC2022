#include <uart.h>
#include <string.h>

void exception_entry(unsigned long what, unsigned long esr, unsigned long elr, unsigned long spsr, unsigned long far, unsigned long type){
    char buf[10];
    uart_puts("---------Exception Handler---------\n");
    uart_puts("[*] Exception type: ");
    switch(type) {
        case 0: uart_puts("Synchronous"); break;
        case 1: uart_puts("IRQ"); break;
        case 2: uart_puts("FIQ"); break;
        case 3: uart_puts("SError"); break;
    }
    uart_puts("\n");

    uart_puts("[*] esr: ");
    uitohex(buf, esr);
    uart_puts(buf);
    uart_puts("\n");

    uart_puts("[*] elr: ");
    uitohex(buf, elr);
    uart_puts(buf);
    uart_puts("\n");

    uart_puts("[*] spsr: ");
    uitohex(buf, spsr);
    uart_puts(buf);
    uart_puts("\n");

    uart_puts("[*] far: ");
    uitohex(buf, far);
    uart_puts(buf);
    uart_puts("\n");
}
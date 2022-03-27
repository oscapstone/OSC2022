#include <uart.h>
#include <string.h>

void exception_entry(unsigned long long what, 
                    unsigned long long esr, 
                    unsigned long long elr, 
                    unsigned long spsr, 
                    unsigned long long type){
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
    uart_puts("[*] spsr_el1: 0x");
    uitohex(buf, spsr);
    uart_puts(buf);
    uart_puts("\n");

    uart_puts("[*] elr_el1: 0x");
    uitohex(buf, elr);
    uart_puts(buf);
    uart_puts("\n");


    uart_puts("[*] esr_el1: 0x");
    uitohex(buf, esr);
    uart_puts(buf);
    uart_puts("\n");

    if (type == 1) {asm volatile("bl core_timer_handler");}
}
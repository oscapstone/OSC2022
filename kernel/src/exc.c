#include <uart.h>
#include <irq.h>
#include <string.h>

void exception_entry(unsigned long long what, 
                    unsigned long long esr, 
                    unsigned long long elr, 
                    unsigned long long spsr, 
                    unsigned long long type){
    char buf[10];
                        
    switch(type) {
        case 0: 
            Sync_exception(esr, elr, spsr);
            break;
        case 1: 
            if(*CORE0_IRQ_SOURCE == 0x2) Time_interrupt(spsr);
            else if(*CORE0_IRQ_SOURCE == 0x100) GPU_interrupt();
            break;
        case 2: uart_puts("FIQ"); break;
        case 3: uart_puts("SError"); break;
    }
    


    // if (type == 1) {
    //     asm volatile("bl core_timer_handler");
    // }
}

void Sync_exception(unsigned long long esr, unsigned long long elr, unsigned long long spsr){
    char buf[10];
    uart_puts("---------Exception Handler---------\n");
    uart_puts("[*] Exception type: ");
    uart_puts("Synchronous");

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
}

void Time_interrupt(unsigned long long spsr){
    spsr &= 0x1111;
    if(spsr == 0x0){
        uart_puts("Time interrupt\n");
    }

    /* reset timer */
    asm volatile(
        "mrs x0, cntfrq_el0\n\t"
        "msr cntp_tval_el0, x0\n\t"
        ::: "x0"
    );
}

void GPU_interrupt(){
    while(!(*AUX_MU_LSR & 0x01)) {asm volatile("nop");}
    /* read it and return */
    char r = (char)(*AUX_MU_IO);
    /* convert carrige return to newline */
    r == '\r'?'\n':r;

    uart_send(r);
}
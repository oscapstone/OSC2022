#include <uart.h>
#include <irq.h>
#include <string.h>
#include <exc.h>
#include <timer.h>
#include <task.h>

void exception_entry(unsigned long long what, 
                    unsigned long long esr, 
                    unsigned long long elr, 
                    unsigned long long spsr, 
                    unsigned long long type){
                        
    switch(type) {
        case 0: 
            Sync_exception(esr, elr, spsr);
            break;
        case 1: 
            if(*CORE0_IRQ_SOURCE & 0x2) Time_interrupt(spsr);
            else if(*CORE0_IRQ_SOURCE & 0x100) GPU_interrupt();
            break;
        case 2: uart_puts("FIQ"); break;
        case 3: uart_puts("SError"); break;
    }
    

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
    unsigned long long frq = 0;
    asm volatile("mrs %0, cntfrq_el0\n\t" :"=r"(frq));
    spsr &= 0b1111;
    if(spsr == 0x0){
        uart_puts("Time interrupt\n");
        set_period_timer_irq();
    }
    else{
        disable_timer_irq();
        add_task(timer_interrupt_handler, 1);
        do_task();
    }
}


/* 
    AUX_MU_IIR:
    On read this register shows the interrupt ID bit Interrupt ID
    bit[2:1]
    00 : No interrupts
    01 : Transmit holding register empty
    10 : Receiver holds valid byte
    11 : <Not possible>

    AUX_MU_IER_REG:
    bit 1: Enable Transmit interrupt
    bit 0: Enable Receive interrupt
*/


void GPU_interrupt(){
    if(*AUX_MU_IIR & TRANSMIT_HOLDING){ // Transmit interrupt
        disable_AUX_MU_IER_w();
        add_task(tran_interrupt_handler, 3);
        do_task();
    }
    else if(*AUX_MU_IIR & RECEIVE_VALID){ // Receive interrupt
        disable_AUX_MU_IER_r();
        disable_AUX_MU_IER_w();
        add_task(recv_interrupt_handler, 2);
        do_task();
    }
    else{
        uart_puts("[*] AUX_MU_IIR: No interrupts\n");
    }
}


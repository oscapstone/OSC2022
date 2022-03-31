/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "exception.h"
#include "address.h"
#include "uart.h"
#include "utils.h"
#include "timer.h"
#include "interrupt.h"


void raise_exc()
{
    asm volatile ("svc 0");
}

void el1_to_el0(char* program_address, char* user_stack_address)
{
    asm volatile(
        "msr elr_el1, %0\n\t"
        // "mov x1, 0x3c0\n\t"
        // "msr spsr_el1, x1\n\t" // disable interrupt in EL0
        "msr spsr_el1, xzr\n\t" // enable interrupt in EL0
        "msr sp_el0, %1\n\t"    
        "eret\n\t"
        :: "r" (program_address),
        "r" (user_stack_address)
    );
}

void exc_dump(unsigned long num, unsigned long esr, unsigned long elr, unsigned long spsr, unsigned long type) {
    uart_async_puts("Number: ");
    uart_async_num(num);
    uart_async_newline();
    switch(type) {
        case 0: uart_async_puts("Synchronous"); break;
        case 1: uart_async_puts("IRQ"); break;
        case 2: uart_async_puts("FIQ"); break;
        case 3: uart_async_puts("SError"); break;
    }
    uart_async_puts(": ");
    // decode exception type (some, not all. See ARM DDI0487B_b chapter D10.2.28)
    switch(esr>>26) {
        case 0b000000: uart_async_puts("Unknown"); break;
        case 0b000001: uart_async_puts("Trapped WFI/WFE"); break;
        case 0b001110: uart_async_puts("Illegal execution"); break;
        case 0b010101: uart_async_puts("System call"); break;
        case 0b100000: uart_async_puts("Instruction abort, lower EL"); break;
        case 0b100001: uart_async_puts("Instruction abort, same EL"); break;
        case 0b100010: uart_async_puts("Instruction alignment fault"); break;
        case 0b100100: uart_async_puts("Data abort, lower EL"); break;
        case 0b100101: uart_async_puts("Data abort, same EL"); break;
        case 0b100110: uart_async_puts("Stack alignment fault"); break;
        case 0b101100: uart_async_puts("Floating point"); break;
        default: uart_async_puts("Unknown"); break;
    }
    // decode data abort cause
    if(esr>>26==0b100100 || esr>>26==0b100101) {
        uart_async_puts(", ");
        switch((esr>>2)&0x3) {
            case 0: uart_async_puts("Address size fault"); break;
            case 1: uart_async_puts("Translation fault"); break;
            case 2: uart_async_puts("Access flag fault"); break;
            case 3: uart_async_puts("Permission fault"); break;
        }
        switch(esr&0x3) {
            case 0: uart_async_puts(" at level 0"); break;
            case 1: uart_async_puts(" at level 1"); break;
            case 2: uart_async_puts(" at level 2"); break;
            case 3: uart_async_puts(" at level 3"); break;
        }
    }
    // dump registers
    uart_async_puts(":\n  ESR_EL1 ");
    uart_async_hex(esr>>32);
    uart_async_hex(esr);
    uart_async_puts(" ELR_EL1 ");
    uart_async_hex(elr>>32);
    uart_async_hex(elr);
    uart_async_puts("\n SPSR_EL1 ");
    uart_async_hex(spsr>>32);
    uart_async_hex(spsr);
    uart_async_puts("\n");
}

void uart_dump() {
    //https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf p13
    /*
        AUX_MU_IIR
        on read bits[2:1] :
        00 : No interrupts
        01 : Transmit holding register empty
        10 : Receiver holds valid byte
        11: <Not possible> 
    */

    // buffer read, write
    if (*AUX_MU_IIR & (0b01 << 1)) //can write
    {
        // uart_puts("Interrupt-UART TX\n");
        disable_uart_w_interrupt();
        add_interrupt(uart_interrupt_w_handler, UART_INTERRUPT_TX_PRIORITY);
        // uart_interrupt_w_handler();
        // enable_uart_w_interrupt();
    }
    else if (*AUX_MU_IIR & (0b10 << 1)) // can read
    {
        // uart_puts("Interrupt-UART RX\n");
        disable_uart_r_interrupt();        
        add_interrupt(uart_interrupt_r_handler, UART_INTERRUPT_RX_PRIORITY);
        // uart_interrupt_r_handler();
        // enable_uart_r_interrupt();
    }
    else
    {
        uart_puts("UART Interrupt error\n");
    }
}

void irq_dump(){
   
    if(*IRQ_PENDING_1 & IRQ_PENDING_1_AUX_INT && *CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_GPU) // from aux && from GPU0 -> uart exception  
    {
        uart_dump();
    }
    else if(*CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_CNTPNSIRQ)  //from CNTPNS (core_timer)
    {
        // uart_puts("Interrupt-Timer\n");
        disable_core_timer();
        add_interrupt(core_timer_handler, TIMER_INTERRUPT_PRIORITY);
        // core_timer_handler();
        // enable_core_timer();
    }
    else {
        uart_puts("No support this irq\n");
    }
}
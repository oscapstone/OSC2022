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
#include "type.h"
#include "syscall.h"
#include "thread.h"
#include "signal.h"
#include "mmu.h"


void raise_exc() {
    asm volatile ("svc 0");
}

void el1_to_el0(char* program_address, char* user_stack_address) {
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

void svc_handle(trapFrame_t *frame, uint64 x1) {
    uint64 mode = frame->x8;
    uint64 *returnValue = &frame->x0;

    // MMU page fault
    esr_el1_t *esr;
    esr = (esr_el1_t *)&x1;
    if (esr->ec == DATA_ABORT_LOWER) {
        raiseError("DATA_ABORT_LOWER Fault\n");
        // handle_abort(esr);
        return;
    }
    else if(esr->ec == INS_ABORT_LOWER) {
        raiseError("INS_ABORT_LOWER Fault\n");
        // handle_abort(esr);
        return;
    }

    // uart_printf("SVC Handle Mode = %d\n", mode); //delay_ms(100);
    enable_interrupt();
    switch(mode) {
        case 0: *returnValue = getpid(); break;
        case 1: *returnValue = uart_read((char *)frame->x0, (size_t)frame->x1); break;
        case 2: *returnValue = uart_write((char *)frame->x0, (size_t)frame->x1); break;
        case 3: *returnValue = exec(frame, (char *)frame->x0, (char **)frame->x1); break;
        case 4: *returnValue = fork(frame);
            // uart_printf("Return %d Done\n", *returnValue);
            break;
        case 5: exit((int)frame->x0); break;
        case 6: *returnValue = mbox_call((unsigned char)frame->x0, (unsigned int *)frame->x1); break;
        case 7: kill((int)frame->x0); break;
        
        case 8: signal_register(frame->x0, (void (*)())frame->x1); break;
        case 9: signal_kill(frame->spsr_el1, frame->x0, frame->x1); break;
        case 115: signal_return(frame); break;
        
        case 11: *returnValue = call_vfs_open((char*)frame->x0, frame->x1); break;
        case 12: *returnValue = call_vfs_close(frame->x0); break;
        case 13: *returnValue = call_vfs_write(frame->x0, (char*)frame->x1, frame->x2); break;
        case 14: *returnValue = call_vfs_read(frame->x0, (char*)frame->x1, frame->x2); break;
        case 15: *returnValue = call_vfs_mkdir((char*)frame->x0, frame->x1); break;
        case 16: *returnValue = call_vfs_mount((char*)frame->x0, (char*)frame->x1, (char*)frame->x2, frame->x3, (void*)frame->x4); break;
        case 17: *returnValue = call_vfs_chdir((char*)frame->x0); break;
        case 18: *returnValue = call_vfs_lseek64(frame->x0, frame->x1, frame->x2); break;
        case 19: *returnValue = call_vfs_ioctl(frame->x0, frame->x1, (void*)frame->x2); break;

        default: break;
    }

    // if(mode == 4) {
        // uart_printf("SVC Handle Mode %d Done: 0x%x\n", mode, *returnValue);
    // }
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

    delay_ms(10000);
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

    // buffer write
    if (*AUX_MU_IIR & (0b01 << 1)) {
        // uart_printf("UART TX Interrupt\n");
        disable_uart_w_interrupt();
        add_interrupt(uart_interrupt_w_handler, UART_INTERRUPT_TX_PRIORITY);
        // uart_printf("UART TX Done\n");
    }
    // buffer read
    else if (*AUX_MU_IIR & (0b10 << 1)) {
        // uart_printf("UART RX Interrupt\n");
        disable_uart_r_interrupt();        
        add_interrupt(uart_interrupt_r_handler, UART_INTERRUPT_RX_PRIORITY);
        // uart_printf("UART RX Done\n");
    }
    else {
        uart_puts("UART Interrupt error\n");
    }
}

void irq_dump(trapFrame_t *frame){
   // from aux && from GPU0 -> uart exception  
    if(*IRQ_PENDING_1 & IRQ_PENDING_1_AUX_INT && *CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_GPU) {
        // uart_printf("UART Interrupt\n");
        uart_dump();
    }
    //from CNTPNS (core_timer)
    else if(*CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_CNTPNSIRQ) {
        disable_core_timer();
        // uart_printf("Timer Interrupt\n");
        add_interrupt(core_timer_handler, TIMER_INTERRUPT_PRIORITY);
        if (run_queue->next->next != run_queue) schedule();
        // uart_printf("Timer Add Interrupt Done\n");
    }
    else {
        uart_puts("No support this irq\n");
    }

    if ((frame->spsr_el1 & 0b1111) == 0) {;
        signal_execute();
    }
}
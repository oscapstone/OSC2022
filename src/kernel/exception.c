#include <uart.h>
#include <string.h>
#include <stdint.h>
#include <exception.h>
#include <interrupt.h>
#include <sched.h>
#include <syscall.h>

void exception_vector_table_init()
{
    //asm("adr x0, %0"::"r"((void*)&exception_vector_table));
    //asm("msr vbar_el1, x0");
    asm("msr vbar_el1, %0"::"r"((void*)&exception_vector_table));
}

void exception_handler(void *sp)
{
    interrupt_disable();
    uint64_t spsr_el1;
    uint64_t elr_el1;
    uint64_t esr_el1;
    uint64_t x0;
    asm("mov %0, x0":"=r"(x0));
    asm("mrs %0, spsr_el1":"=r"(spsr_el1));
    asm("mrs %0, elr_el1":"=r"(elr_el1));
    asm("mrs %0, esr_el1":"=r"(esr_el1));
    // uart_print("x0: ");
    // uart_putshex(x0);
    // uart_print("spsr_el1: ");
    // uart_putshex(spsr_el1);
    // uart_print("elr_el1: ");
    // uart_putshex(elr_el1);
    // uart_print("esr_el1: ");
    // uart_putshex(esr_el1);
    if(esr_el1 >> 26 == 0b010101){
        // uart_puts("svc interrupt");
        uint16_t svcnum = 0;
        svcnum = esr_el1 & 0xffff;
        if(svcnum==0){
            interrupt_enable();
            syscall_handler(sp);
        }
        else if(svcnum==1){
            interrupt_enable();
            schedule();
        }
    }
    interrupt_enable();
}
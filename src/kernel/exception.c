#include <uart.h>
#include <string.h>
#include <stdint.h>
#include <exception.h>
#include <interrupt.h>
#include <sched.h>
#include <syscall.h>
#include <mmu.h>
#include <error.h>

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
    uint64_t far_el1;
    uint64_t x0;
    asm("mov %0, x0":"=r"(x0));
    asm("mrs %0, spsr_el1":"=r"(spsr_el1));
    asm("mrs %0, elr_el1":"=r"(elr_el1));
    asm("mrs %0, esr_el1":"=r"(esr_el1));
    asm("mrs %0, far_el1":"=r"(far_el1));
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
        //((TrapFrame*)sp)->spsr_el1 = 0;
        if(svcnum==0){
            interrupt_enable();
            syscall_handler(sp);
        }
        else if(svcnum==1){
            interrupt_enable();
            schedule();
        }
    }
    else if(esr_el1 >> 26 == 0b100000 || esr_el1 >> 26 == 0b100100){ //Abort from a lower Exception level.
        // page fault
        uint64_t fsc = esr_el1 & 0b111111;
        kmsg("Page Fault: elr_el1=0x%x, esr_el1=0x%x, fsc=0x%x, far_el1=0x%x", elr_el1, esr_el1, fsc, far_el1);
        //kmsg("fsc & 0b1100 = 0x%x", fsc&0b1100);
        if((fsc & 0b1100) == 0b0100){ //Translation fault (demand paging)
            kmsg("call mmu_page_fault_handler()");
            mmu_page_fault_handler(far_el1);
        }
        else if((fsc & 0b1100) == 0b1100){ //Permission fault (Copy on Write)
            mmu_page_permission_handler(far_el1, esr_el1);
        }
    }
    else if(esr_el1 >> 26 == 0b100001 || esr_el1 >> 26 == 0b100101){ //Abort taken without a change in Exception level.
        //kpanic("Page Fault at kernel. 0x%x", far_el1);
        kpanic("Page Fault at kernel: elr_el1=0x%x, esr_el1=0x%x, far_el1=0x%x", elr_el1, esr_el1, far_el1);
    }
    else{
        kmsg("Exception: elr_el1=0x%x, esr_el1=0x%x, far_el1=0x%x", elr_el1, esr_el1, far_el1);
    }
    
    interrupt_enable();
}
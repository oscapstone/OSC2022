#include "uart.h"
#include "exception.h"
#include "timer.h"

void sync_64_router(unsigned long long x0){
    unsigned long long spsr_el1;
	__asm__ __volatile__("mrs %0, SPSR_EL1\n\t" : "=r" (spsr_el1) :  : "memory");

    unsigned long long elr_el1;
	__asm__ __volatile__("mrs %0, ELR_EL1\n\t" : "=r" (elr_el1) :  : "memory");

    unsigned long long esr_el1;
	__asm__ __volatile__("mrs %0, ESR_EL1\n\t" : "=r" (esr_el1) :  : "memory");

    uart_printf("exception sync_el0_64_router -> spsr_el1 : 0x%x, elr_el1 : 0x%x, esr_el1 : 0x%x\r\n",spsr_el1,elr_el1,esr_el1);

}

void irq_router(unsigned long long x0){
    //uart_printf("exception type: %x\n",x0);
    //uart_printf("irq_basic_pending: %x\n",*IRQ_BASIC_PENDING);
    //uart_printf("irq_pending_1: %x\n",*IRQ_PENDING_1);
    //uart_printf("irq_pending_2: %x\n",*IRQ_PENDING_2);
    //uart_printf("source : %x\n",*CORE0_INTERRUPT_SOURCE);

    //目前實測能從pending_1 AUX_INT, CORE0_INTERRUPT_SOURCE=GPU 辨別其他都是0(或再找)
    if(*IRQ_PENDING_1 & IRQ_PENDING_1_AUX_INT && *CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_GPU) // from aux && from GPU0 -> uart exception  
    {
        uart_interrupt_handler();
    }else if(*CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_CNTPNSIRQ)  //from CNTPNS (core_timer)
    {
        core_timer_handler();
    }
}

void invalid_exception_router(unsigned long long x0){
    uart_printf("invalid exception\n : %d",x0);
}

void enable_interrupt(){
    __asm__ __volatile__("msr daifclr, 0xf");
}

void disable_interrupt(){
    __asm__ __volatile__("msr daifset, 0xf");
}
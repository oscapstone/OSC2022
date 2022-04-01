#include "printf.h"
#include "exception.h"
#include "timer.h"
#include "uart.h"

void invalid_exception_router(unsigned long long x0){
  unsigned long long elr_el1, esr_el1, spsr_el1;
  __asm__ __volatile__("mrs %[output0], elr_el1\n\t"
                       "mrs %[output1], ESR_EL1\n\t"
                       "mrs %[output2], SPSR_EL1\n\t"
                       : [output0] "=r" (elr_el1), [output1] "=r" (esr_el1), [output2] "=r" (spsr_el1)
                       :
                       : );
  printf("elr_el1 : 0x%x\r\n", elr_el1);
  printf("esr_el1 : 0x%x\r\n", esr_el1);
  printf("spsr_el1 : 0x%x\r\n", spsr_el1);
  printf("exception number: 0x%x\r\n",x0);
  while(1);
}

void irq_router(unsigned long long x0){
  if(*CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_CNTPNSIRQ){
    core_timer_interrupt_disable();
    pop_timer();
    pop_task();
  }else if(*IRQS1_PENDING & (0x01 << 29)){
    if (*AUX_MU_IIR & (0b01 << 1)) {  //can write
      disable_uart_w_interrupt();
      add_task(uart_interrupt_w_handler, "TX", 0);
      pop_task();
      // uart_interrupt_w_handler();
    }else if (*AUX_MU_IIR & (0b10 << 1)) {  //can read
      disable_uart_r_interrupt();
      add_task(uart_interrupt_r_handler, "RX", 0);
      pop_task();
      // uart_interrupt_r_handler();
    }
  }
}

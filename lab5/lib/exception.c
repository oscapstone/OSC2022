#include "printf.h"
#include "exception.h"
#include "timer.h"
#include "uart.h"
#include "scheduler.h"
#include "cpio.h"

void invalid_exception_router(uint64_t x0){
  uint64_t elr_el1, esr_el1, spsr_el1;
  __asm__ __volatile__("mrs %[output0], elr_el1\n\t"
                       "mrs %[output1], ESR_EL1\n\t"
                       "mrs %[output2], SPSR_EL1\n\t"
                       : [output0] "=r" (elr_el1), [output1] "=r" (esr_el1), [output2] "=r" (spsr_el1)
                       :
                       : );
  char str[20] = "";                 
  sprintf(str, "elr_el1 : 0x%x\r\n", elr_el1);
  uart_puts(str);
  sprintf(str, "esr_el1 : 0x%x\r\n", esr_el1);
  uart_puts(str);
  sprintf(str, "spsr_el1 : 0x%x\r\n", spsr_el1);
  uart_puts(str);
  sprintf(str, "exception number: 0x%x\r\n",x0);
  uart_puts(str);
  // while(1);
}

void irq_router(uint64_t x0){
  if(*CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_CNTPNSIRQ){
    core_timer_interrupt_disable();
    pop_timer();
  }else if(*IRQS1_PENDING & (0x01 << 29)){
    if (*AUX_MU_IIR & (0b01 << 1)) {  //can write
      disable_uart_w_interrupt();
      uart_interrupt_w_handler();
    }else if (*AUX_MU_IIR & (0b10 << 1)) {  //can read
      disable_uart_r_interrupt();
      uart_interrupt_r_handler();
    }
  }
}

void sync_excep(uint64_t x0, uint64_t x1){
  trap_frame *frame = (trap_frame *)x1;
  if(frame->x8 == 0){
    task *cur = get_current();
    frame->x0 = cur->pid;
  }else if(frame->x8 == 1){
    int read = 0;
    char *buf = (char *)frame->x0;
    while (read < frame->x1){
      *(buf+read) = uart_getc();
      read++;
    }
    frame->x0 = read;
  }else if(frame->x8 == 2){
    int sent = 0;
    char *buf = (char *)frame->x0;
    while (sent < frame->x1){
      uart_send(*buf++);
      sent++;
    }
    frame->x0 = sent;
  }else if(frame->x8 == 3){
    char *name = (char *)frame->x0;
    // char *argv = (char *)frame->x1;
    cpio_exec(name);
    frame->x0 = 0;
  }else if(frame->x8 == 5){
    task *cur = get_current();
    cur->state = EXIT;
  }
}

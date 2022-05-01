#include "printf.h"
#include "exception.h"
#include "timer.h"
#include "uart.h"
#include "scheduler.h"
#include "cpio.h"
#include "mailbox.h"

void invalid_exception_router(uint64_t x0){
  uint64_t elr_el1, esr_el1, spsr_el1;
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
}

void irq_router(uint64_t x0){
  if(*IRQS1_PENDING & (0x01 << 29)){
    if (*AUX_MU_IIR & (0b01 << 1)) {  //can write
      disable_uart_w_interrupt();
      uart_interrupt_w_handler();
    }else if (*AUX_MU_IIR & (0b10 << 1)) {  //can read
      disable_uart_r_interrupt();
      uart_interrupt_r_handler();
    }
  }else if(*CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_CNTPNSIRQ){
    interrupt_disable();
    core_timer_interrupt_disable();
    pop_timer();
  }
}

void sync_router(uint64_t x0, uint64_t x1){
  // printf("sync_router %d\n\r", x0);
  interrupt_disable();
  trap_frame *frame = (trap_frame *)x1;
  if(frame->x8 == 0){               // get pid
    task *cur = get_current();
    frame->x0 = cur->pid;
  }else if(frame->x8 == 1){         // uart read
    int read = 0;
    char *buf = (char *)frame->x0;
    while (read < frame->x1){
      *(buf+read) = uart_getc();
      read++;
    }
    frame->x0 = read;
  }else if(frame->x8 == 2){         // uart write
    int sent = 0;
    char *buf = (char *)frame->x0;
    while (sent < frame->x1){
      uart_send(*buf++);
      sent++;
    }
    frame->x0 = sent;
  }else if(frame->x8 == 3){         // exec
    char *name = (char *)frame->x0;
    task *cur = get_current();
    frame->sp_el0 = cur->user_sp;
    char *addr = load_program(name);
    frame->elr_el1 = (uint64_t)addr;
    frame->x8 = 8;
    // char *argv = (char *)frame->x1;
    frame->x0 = 0;
  }else if(frame->x8 == 4){        // fork
    printf("this is fork\r\n");
    task *parent = get_current();
    task *child = task_create(NULL, USER);
    
    /* copy the task context & kernel stack (including trap frame) of parent to child */
    child->x19 = parent->x19;
    child->x20 = parent->x20;
    child->x21 = parent->x21;
    child->x22 = parent->x22;
    child->x23 = parent->x23;
    child->x24 = parent->x24;
    child->x25 = parent->x25;
    child->x26 = parent->x26;
    child->x27 = parent->x27;
    child->x28 = parent->x28;
    child->fp = parent->fp;
    child->lr = frame->x30;
    child->sp = parent->sp;
    child->target_func = parent->target_func;
    // copy the stack
    char *src1 = (char *)parent->user_sp;
    char *dst1 = (char *)child->user_sp;
    char *src2 = (char *)parent->sp_addr;
    char *dst2 = (char *)child->sp_addr;
    for(int i=0; i<THREAD_SP_SIZE; i++){
      *(dst1+i) = *(src1+i);
      *(dst2+i) = *(src2+i); 
    }
    // if((uint64_t)child->user_sp > (uint64_t)parent->user_sp){
    //   child->sp = child->sp + ((uint64_t)child->user_sp - (uint64_t)parent->user_sp);
    //   child->fp = child->fp + ((uint64_t)child->user_sp - (uint64_t)parent->user_sp);
    // }else if((uint64_t)child->user_sp < (uint64_t)parent->user_sp){
    //   child->sp = child->sp - ((uint64_t)parent->user_sp - (uint64_t)child->user_sp);
    //   child->fp = child->fp - ((uint64_t)parent->user_sp - (uint64_t)child->user_sp);
    // }
    if((uint64_t)child->sp_addr > (uint64_t)parent->sp_addr){
      child->sp = child->sp + ((uint64_t)child->sp_addr - (uint64_t)parent->sp_addr);
      child->fp = child->fp + ((uint64_t)child->sp_addr - (uint64_t)parent->sp_addr);
    }else if((uint64_t)child->sp_addr < (uint64_t)parent->sp_addr){
      child->sp = child->sp - ((uint64_t)parent->sp_addr - (uint64_t)child->sp_addr);
      child->fp = child->fp - ((uint64_t)parent->sp_addr - (uint64_t)child->sp_addr);
    }
    frame->x0 = child->pid;
  }else if(frame->x8 == 5){        // exit
    task *cur = get_current();
    printf("exit %d\n\r", cur->pid);
    cur->state = EXIT;
    schedule();
  }else if(frame->x8 == 6){        // mbox call
    unsigned char ch = (unsigned char)frame->x0;
    uint32_t *mbox = (uint32_t *)frame->x1;
    frame->x0 = mbox_call(ch, mbox);
  }else if(frame->x8 == 7){        // kill
    kill_thread(frame->x0);
  }
  interrupt_enable();
}

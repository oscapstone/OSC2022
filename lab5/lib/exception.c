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
    task *parent = get_current();
    task *child = task_create(NULL, USER);
    printf("this is fork %d->%d\r\n", parent->pid, child->pid);
    /* copy the task context & kernel stack (including trap frame) of parent to child */
    child->x19 = frame->x19;
    child->x20 = frame->x20;
    child->x21 = frame->x21;
    child->x22 = frame->x22;
    child->x23 = frame->x23;
    child->x24 = frame->x24;
    child->x25 = frame->x25;
    child->x26 = frame->x26;
    child->x27 = frame->x27;
    child->x28 = frame->x28;
    child->fp = frame->x29;
    child->lr = (uint64_t)child_return_from_fork; // frame->x30;
    child->sp = (uint64_t)frame; // + sizeof(trap_frame);
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
    if((uint64_t)child->sp_addr > (uint64_t)parent->sp_addr){
      child->sp += ((uint64_t)child->sp_addr - (uint64_t)parent->sp_addr);
      child->fp += ((uint64_t)child->sp_addr - (uint64_t)parent->sp_addr);
    }else if((uint64_t)child->sp_addr < (uint64_t)parent->sp_addr){
      child->sp -= ((uint64_t)parent->sp_addr - (uint64_t)child->sp_addr);
      child->fp -= ((uint64_t)parent->sp_addr - (uint64_t)child->sp_addr);
    }
    trap_frame *child_frame = (trap_frame *)child->sp;
    child_frame->x0 = 0;
    child_frame->x29 = child->fp;
    // printf("p->user_sp: 0x%x, f->sp_el0: 0x%x\n\r", parent->user_sp, frame->sp_el0);
    // printf("p->sp_addr: 0x%x, f: 0x%x\n\r", parent->sp_addr, frame);
    if((uint64_t)child->user_sp > (uint64_t)parent->user_sp){
      child_frame->sp_el0 += ((uint64_t)child->user_sp - (uint64_t)parent->user_sp);
    }else if((uint64_t)child->user_sp < (uint64_t)parent->user_sp){
      child_frame->sp_el0 -= ((uint64_t)parent->user_sp - (uint64_t)child->user_sp);
    }
    // printf("c->sp_addr: 0x%x, c_tf: 0x%x\n\r", child->sp_addr, child_frame);
    // printf("c->user_sp: 0x%x, c->sp_el0: 0x%x\n\r", child->user_sp, child_frame->sp_el0);
    // printf("p->lr: 0x%x, c->lr: 0x%x\n\r", frame->x30, child->lr);
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

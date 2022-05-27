#include "printf.h"
#include "exception.h"
#include "timer.h"
#include "uart.h"
#include "scheduler.h"
#include "cpio.h"
#include "mailbox.h"
#include "system.h"
#include "mmu.h"
#include "malloc.h"

#define read_sysreg(r) ({                       \
    unsigned long __val;                        \
    asm volatile("mrs %0, " #r : "=r" (__val)); \
    __val;                                      \
})

static void signal_handler_wrapper();
static handler_func _handler = NULL;
static uint64_t _pid = 0;

void invalid_exception_router(uint64_t x0){
  uint64_t elr_el1, esr_el1, spsr_el1;
  asm volatile("mrs %[output0], elr_el1  \n" :[output0] "=r" (elr_el1) );
  asm volatile("mrs %[output1], ESR_EL1  \n" :[output1] "=r" (esr_el1) );
  asm volatile("mrs %[output2], SPSR_EL1 \n" :[output2] "=r" (spsr_el1));  
  printf("exception number: 0x%x\r\n",x0);
  printf("elr_el1: 0x%x\r\n", elr_el1);
  printf("esr_el1: 0x%x\r\n", esr_el1);
  printf("spsr_el1: 0x%x\r\n", spsr_el1);
  while(1);
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

void sync_router(uint64_t x0, uint64_t x1){
  trap_frame *frame = (trap_frame *)x1;
  if(frame->x8 == 0){               // get pid
    task *cur = get_current();
    frame->x0 = cur->pid;
  }else if(frame->x8 == 1){         // uart read
    interrupt_enable();
    char *buf = (char *)frame->x0;
    for(int i=0; i < frame->x1; i++){
      buf[i] = uart_getc();
    }
    frame->x0 = frame->x1;
    interrupt_disable();
  }else if(frame->x8 == 2){         // uart write
    interrupt_enable();
    char *buf = (char *)frame->x0;
    for(int i=0; i<frame->x1; i++)
      uart_send(buf[i]);
    frame->x0 = frame->x1;
    interrupt_disable();
  }else if(frame->x8 == 3){         // exec
    char *name = (char *)frame->x0;
    task *cur = get_current();
    init_PT(&(cur->page_table));
    load_program(name, cur->page_table);
    for (int i = 0; i < 4; i++)
      map_pages(cur->page_table, 0xffffffffb000 + i*0x1000, VA2PA(page_allocate_addr(0x1000)));
    frame->sp_el0 = 0xfffffffff000 - 0x10;
    frame->elr_el1 = (uint64_t)USER_PROGRAM_ADDR;
    // char *argv = (char *)frame->x1;
    asm volatile("mov x0, %0 			\n"::"r"(cur->page_table));
    asm volatile("dsb ish 	\n");             // ensure write has completed
	  asm volatile("msr ttbr0_el1, x0 	\n");   // switch translation based address.
    asm volatile("tlbi vmalle1is 	\n");       // invalidates cached copies of translation table entries from L1 TLBs
    asm volatile("dsb ish 	\n");             // ensure completion of TLB invalidatation
    asm volatile("isb 	\n");                 // clear pipeline
    frame->x0 = 0;
  }else if(frame->x8 == 4){        // fork
    // printf("fork\n\r");
    task *parent = get_current();
    task *child = task_create(NULL, USER);
    duplicate_PT(parent->page_table, child->page_table);
    char *src1 = (char *)(parent->user_sp);
    char *dst1 = (char *)(child->user_sp);
    for(int i=0; i<0x4000; i++){
      dst1[i] = src1[i];
    }
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
    child->lr = (uint64_t)child_return_from_fork;
    child->sp = (uint64_t)frame;
    child->target_func = parent->target_func;
    child->handler = parent->handler;

    // copy the stack in el1
    char *src2 = (char *)parent->sp_addr;
    char *dst2 = (char *)child->sp_addr;
    for(int i=0; i<0x1000; i++){
      dst2[i] = src2[i];
    }
    // shift the sp_el1
    if((uint64_t)child->sp_addr > (uint64_t)parent->sp_addr){
      child->sp += ((uint64_t)child->sp_addr - (uint64_t)parent->sp_addr);
    }else if((uint64_t)child->sp_addr < (uint64_t)parent->sp_addr){
      child->sp -= ((uint64_t)parent->sp_addr - (uint64_t)child->sp_addr);
    }

    trap_frame *child_frame = (trap_frame *)child->sp;
    child_frame->x0 = 0;
    frame->x0 = child->pid;
  }else if(frame->x8 == 5){        // exit
    task *cur = get_current();
    cur->state = EXIT;
    schedule();
  }else if(frame->x8 == 6){        // mbox call
    // printf("mailbox\n\r");
    unsigned char ch = (unsigned char)frame->x0;
    uint32_t *mbox = (uint32_t *)frame->x1;
    asm volatile("mov x0, %0    \n"::"r"(mbox));
    asm volatile("at s1e0r, x0  \n");
    uint64_t frame_addr = (uint64_t)read_sysreg(par_el1) & (0xFFFFFFFFF << 12);
    uint64_t pa = frame_addr | ((uint64_t)mbox & 0xFFF);
    frame->x0 = mbox_call(ch, (unsigned int *)pa, mbox);
  }else if(frame->x8 == 7){        // kill
    kill_thread(frame->x0);
  }else if(frame->x8 == 8){        // register
    task *cur = get_current();
    cur->handler = (void (*)())frame->x1;
  }else if(frame->x8 == 9){       // signal kill
    task *cur = get_current();
    _handler = (handler_func)cur->handler;
    _pid = frame->x0;
    task *handler_task = task_create(NULL, USER);
    handler_task->target_func = (uint64_t)signal_handler_wrapper;
  }
}

void signal_handler_wrapper(){
  if (_handler){
    _handler();
  }
  sys_kill(_pid);
  sys_exit();
}

void ppp(uint64_t x0){
  trap_frame *child = (trap_frame *)x0;
  printf("child->sp_el0: 0x%llx\n\r", child->sp_el0);
  printf("child->elr_el1: 0x%llx\n\r", child->elr_el1);
  printf("child->spsr_el1: 0x%llx\n\r", child->spsr_el1);
}

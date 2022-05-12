#include <stdint.h>
#include "exception.h"
#include "mbox.h"
#include "thread.h" 
#include "io.h"
#include "mini_uart.h"
#include "timer.h"
#include "printf.h"
#include "utils.h"

int count = 0;

void enable_interrupt() { asm volatile("msr DAIFClr, 0xf"); }

void disable_interrupt() { asm volatile("msr DAIFSet, 0xf"); }

void currentEL_ELx_sync_handler() {
  printf("[sync_handler_currentEL_ELx]\n");

  uint64_t spsr_el1, elr_el1, esr_el1;
  asm volatile("mrs %0, spsr_el1" : "=r"(spsr_el1));
  asm volatile("mrs %0, elr_el1" : "=r"(elr_el1));
  asm volatile("mrs %0, esr_el1" : "=r"(esr_el1));
  printf("SPSR_EL1: 0x%08x\n", spsr_el1);
  printf("ELR_EL1: 0x%08x\n", elr_el1);
  printf("ESR_EL1: 0x%08x\n", esr_el1);
  while(1){}
  // 0x96000000 = 10010110000000000000000000000000, MMU fault
}

void lower_64_EL_sync_handler(uint64_t sp){
  // exception registor
  uint64_t spsr_el1, elr_el1, esr_el1;
  asm volatile("mrs %0, spsr_el1" : "=r"(spsr_el1));
  asm volatile("mrs %0, elr_el1" : "=r"(elr_el1));
  asm volatile("mrs %0, esr_el1" : "=r"(esr_el1));

  // https://developer.arm.com/documentation/ddi0595/2021-06/AArch64-Registers/ESR-EL1--Exception-Syndrome-Register--EL1-
  int ec = (esr_el1 >> 26) & 0x3f; // exception class
  if(ec == 0b010101){
    // according the lab spec, the number of the systme call is store in x8
    trap_frame_t *trap_frame = (trap_frame_t *)sp;
    int sys_call_num;
    asm volatile("mov %0, x8" : "=r"(sys_call_num));

    if(sys_call_num == 0){ // int getpid()
      //uint32_t pid = get_current()->pid;
      uint64_t pid = get_current()->pid;
      //printf("ffpid: %d\n", pid);
      trap_frame->x[0]=pid;
      //asm volatile("mov x0, %0" : "=r"(pid)); // function reture value
    }else if(sys_call_num == 1){ // uart_read(char buf[], size_t size)
      //printf("uart_read\n");
      char *str = (char *)(trap_frame->x[0]);
      uint32_t size = (uint32_t)(trap_frame->x[1]);

      disable_uart_interrupt(); // I'm not sure why
      enable_interrupt();
      
      size = uart_gets(str, size);
      trap_frame->x[0] = size;
      //printf("uart_end\n");
      
    }else if(sys_call_num == 2){ // uart_write(char buf[], size_t size)
      //disable_uart_interrupt();
      char* str = (char*) trap_frame->x[0];
      uint32_t size = (uint32_t)(trap_frame->x[1]);
      uart_write(str, size);  
    }else if(sys_call_num == 3){
      print_s("exec called\r\n");
      //char *program_name = (char *)trap_frame->x[0];
      //const char **argv = (const char **)trap_frame->x[1];
      //thread_create(exec);
      exec();
    }else if(sys_call_num == 4){
      print_s("fork called\r\n");
      fork(sp);
    }else if(sys_call_num == 5){
      print_s("exit called\r\n");
      exit();
    }else if(sys_call_num == 6){
      print_s("mbox_call called\r\n");
      unsigned char ch = (unsigned char) trap_frame->x[0];
      unsigned int *user_mbox = (unsigned int*)(trap_frame->x[1]);
      printf("mbox ch: %d, *mbox: %d\n", ch, user_mbox);
      int valid = mbox_call_user(ch, user_mbox);
      printf("mbox call valid: %d\n", valid);
      trap_frame->x[0] = valid;
    }else if(sys_call_num == 7){
      print_s("kill called\r\n");
      int pid = trap_frame->x[0];
      kill(pid);
    }else{
      print_s("unhandled system call number\r\n");
    }
  }
}

// user timer handler
void el0_to_el1_irq_handler() {
  disable_interrupt();
  uint32_t is_uart = (*IRQ_PENDING_1 & AUX_IRQ);
  uint32_t is_core_timer = (*CORE0_IRQ_SOURCE & CNTPNS_IRQ);
  //printf("is_core_timer: %d\n", is_core_timer);
  if (is_uart) {
    uart_handler();
  } else if(is_core_timer) {
    timer_schedular_handler();
  }
  enable_interrupt();
}

// kernel timer handler
uint64_t el1_to_el1_irq_handler(uint64_t sp) {
  disable_interrupt();
  uint32_t is_uart = (*IRQ_PENDING_1 & AUX_IRQ);
  uint32_t is_core_timer = (*CORE0_IRQ_SOURCE & CNTPNS_IRQ);
  //printf("kernel:is_core_timer: %d\n", is_core_timer);
  if (is_uart) {
    uart_handler();
  } else if(is_core_timer) {
    timer_schedular_handler();
  }
  //plan_next_interrupt_tval(SCHEDULE_TVAL);
  enable_interrupt();
  return sp;
}

void default_handler() { print_s("===== default handler =====\n"); }
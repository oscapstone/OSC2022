#include "exception.h"
#include "uart.h"
#include "utils.h"
#include "timer.h"
#include "string.h"
#include "thread.h"
#include "printf.h"
#include "mbox.h"
int count = 0;

void enable_interrupt() { asm volatile("msr DAIFClr, 0xf"); }

void disable_interrupt() { asm volatile("msr DAIFSet, 0xf"); }

void sync_handler_currentEL_ELx() {
  // printf("[sync_handler_currentEL_ELx]\n");

  uint64_t spsr_el1, elr_el1, esr_el1;
  asm volatile("mrs %0, spsr_el1" : "=r"(spsr_el1));
  asm volatile("mrs %0, elr_el1" : "=r"(elr_el1));
  asm volatile("mrs %0, esr_el1" : "=r"(esr_el1));
  // printf("SPSR_EL1: 0x%08x\n", spsr_el1);
  // printf("ELR_EL1: 0x%08x\n", elr_el1);
  // printf("ESR_EL1: 0x%08x\n", esr_el1);
  // printf("hi\n");
}

void sync_handler_lowerEL_64(uint64_t sp) {
  // printf("sync_handler_lowerEL_64 sp : %x\n",sp);
  uint64_t spsr_el1, elr_el1, esr_el1;
  asm volatile("mrs %0, spsr_el1" : "=r"(spsr_el1));
  asm volatile("mrs %0, elr_el1" : "=r"(elr_el1));
  asm volatile("mrs %0, esr_el1" : "=r"(esr_el1));
  // printf("sync, SPSR_EL1: 0x%08x\n", spsr_el1);
  // printf("ELR_EL1: 0x%08x\n", elr_el1);
  // printf("ESR_EL1: 0x%08x\n", esr_el1);

  uint32_t ec = (esr_el1 >> 26) & 0x3f;
  // printf("EC: %x\n", ec);
  if (ec == 0b010101) {  // SVC instruction
    uint64_t iss;
    asm volatile("mov %0, x8" : "=r"(iss));
    // printf("syscall number: %d\n", iss);
    trap_frame_t *trap_frame = (trap_frame_t *)sp;

    if (iss == 0) {  // getpid
      uint32_t pid = get_current()->pid;
      trap_frame->x[0] = pid;
    } else if (iss == 1) {  // uartread
      disable_uart_interrupt();
      enable_interrupt();
      char *str = (char *)(trap_frame->x[0]);
      uint32_t size = (uint32_t)(trap_frame->x[1]);
      size = uart_gets(str, size);
      trap_frame->x[0] = size;
    } else if (iss == 2) {  // uartwrite
      char *str = (char *)(trap_frame->x[0]);
      trap_frame->x[0] = uart_write(str,trap_frame->x[1]);
    } else if (iss == 3) {  // exec
      const char *program_name = (const char *)trap_frame->x[0];
      const char **argv = (const char **)trap_frame->x[1];
      exec(program_name, argv);
    } else if (iss == 4) {  // fork
      // printf("[fork]\n");
      fork(sp);
    } else if (iss == 5) {  // exit
      exit();
    } else if (iss == 6) {  // mbox_call
      // printf("[mbox_call]\n");
      thread_info *cur = get_current();
      unsigned int * mbox_user_va = (unsigned int *)trap_frame->x[1];
      unsigned int * mbox_user_pa = (unsigned int *)el0_VA2PA(cur,(uint64_t)mbox_user_va);
      // printf("mbox_user_va:%p\n",mbox_user_va);
      // printf("mbox_user_pa :%p\n",mbox_user_pa);
      unsigned int * mbox_kernel_va = (unsigned int *) PA2VA(mbox_user_pa);
      // printf("mbox_kernel_va :%p\n",mbox_kernel_va);      // trap_frame->x[0] = mbox_call(trap_frame->x[0],(unsigned int *)trap_frame->x[1]);
      trap_frame->x[0] = mbox_call(trap_frame->x[0],mbox_kernel_va);
    } else if (iss == 7) {  // kill
      kill((int)trap_frame->x[0]);
    } 
  }
}

void irq_handler_currentEL_ELx() {
  // printf("====irq_handler_currentEL_ELx=====\n");

  disable_interrupt();
  uint32_t is_uart = (*IRQ_PENDING_1 & AUX_IRQ);
  uint32_t is_core_timer = (*CORE0_IRQ_SOURCE & 0x2);

  if (is_uart) {
    uart_handler();
  } else if (is_core_timer) {
    core_timer_handler_lowerEL_64();
  }
  enable_interrupt();
}

void irq_handler_lowerEL_64() {
  // printf("====irq_handler_lowerEL_64=====\n");
  disable_interrupt();
  uint32_t is_uart = (*IRQ_PENDING_1 & AUX_IRQ);
  uint32_t is_core_timer = (*CORE0_IRQ_SOURCE & 0x2);

  if (is_uart) {
    uart_handler();
  } else if (is_core_timer) {
    core_timer_handler_lowerEL_64();
  }
  enable_interrupt();
}


void default_handler() { uart_puts("===== default handler =====\n"); }

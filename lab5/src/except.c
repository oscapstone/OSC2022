#include "except.h"
#include "textio.h"
#include "mini_uart.h"
#include <stdint.h>

void set_exception_vector_table() {
  asm volatile("adr x0, exception_vector_table\n\t"
               "msr vbar_el1, x0");
}


void print_exception_info() {
  unsigned long spsr, elr, esr;
  asm volatile("mrs %0, spsr_el1"
               : "=r" (spsr));
  asm volatile("mrs %0, elr_el1"
               : "=r" (elr));
  asm volatile("mrs %0, esr_el1"
               : "=r" (esr));
  kprintf("spsr_el1: %lx\n", spsr);
  kprintf("elr_el1: %lx\n", elr);
  kprintf("esr_el1: %lx\n", esr);
}


void c_irq_handler() {
  volatile uint64_t irq_src = *CORE0_IRQ_SOURCE;
  if (irq_src & (1<<1)) {
    // bit 1 indicates cntpnsirq interrupt
    print("[K] Receive core_timer irq\n");
  } else if (irq_src & (1<<8)) {
    print("[K] Receive GPU irq\n");
  } else {
    kprintf("Invalid interrupt source %lx\n", irq_src);
  }
}

void c_invalid_exception() {
  print("[K] Invalid exception\n");
}

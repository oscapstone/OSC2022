#include "mini_uart.h"
#include "user.h"

void print_exception_info() {
  unsigned long spsr, elr, esr;
  asm volatile("mrs %0, spsr_el1"
               : "=r" (spsr));
  asm volatile("mrs %0, elr_el1"
               : "=r" (elr));
  asm volatile("mrs %0, esr_el1"
               : "=r" (esr));
  print("spsr_el1: ");
  print_hexl(spsr);
  print("\nelr_el1: ");
  print_hexl(elr);
  print("\nesr_el1: ");
  print_hex(esr);
  print_char('\n');
}

void core_timer_handler() {
  print("received timer interrupt!\n");
  uint64_t freq;
  asm volatile("mrs %0, cntfrq_el0"
               : "=r" (freq));
  print("reset timer to ");
  print_num(2*freq);
  print_char('\n');
  reset_timer(2*freq);
}

#include "exception.h"
#include "uart.h"
#include "stdint.h"

void exception_handler() {
  uint64_t spsr_el1, elr_el1, esr_el1;
  asm volatile("mrs %0, spsr_el1" : "=r"(spsr_el1));
  asm volatile("mrs %0, elr_el1" : "=r"(elr_el1));
  asm volatile("mrs %0, esr_el1" : "=r"(esr_el1));
  uart_puts("SPSR_EL1: ");
  uart_hex(spsr_el1);
  uart_puts("\n");
  uart_puts("ELR_EL1: ");
  uart_hex(elr_el1);
  uart_puts("\n");
  uart_puts("ESR_EL1: ");
  uart_hex(esr_el1);
  uart_puts("\n");
}

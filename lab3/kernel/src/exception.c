#include "exception.h"
#include "uart.h"
#include "stdint.h"
#include "timer.h"

int count = 0;

void enable_interrupt() { asm volatile("msr DAIFClr, 0xf"); }

void disable_interrupt() { asm volatile("msr DAIFSet, 0xf"); }

void sync_handler() {
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

void irq_handler_currentEL_ELx() {
  disable_interrupt();
  uint32_t is_uart = (*IRQ_PENDING_1 & AUX_IRQ);
  uint32_t is_core_timer = (*CORE0_INTERRUPT_SOURCE & 0x2);

  if (is_uart) {
    uart_handler();
  } else if (is_core_timer) {
    core_timer_handler_currentEL_ELx();
  }
  enable_interrupt();
}

void irq_handler_lowerEL_64() {
  disable_interrupt();
  uint32_t is_uart = (*IRQ_PENDING_1 & AUX_IRQ);
  uint32_t is_core_timer = (*CORE0_INTERRUPT_SOURCE & 0x2);

  if (is_uart) {
    uart_handler();
  } else if (is_core_timer) {
    core_timer_handler_lowerEL_64();
  }
  enable_interrupt();
}


void default_handler() { uart_puts("===== default handler =====\n"); }

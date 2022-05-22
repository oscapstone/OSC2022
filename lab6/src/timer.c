#include "timer.h"
#include "mini_uart.h"
#include "mmu.h"
#include <stdint.h>

uint64_t get_cpu_freq() {
  static uint64_t cpu_freq;
  if (cpu_freq == 0) {
    volatile uint64_t frq;
    asm volatile("mrs %0, cntfrq_el0" : "=r" (frq));
    cpu_freq = frq;
  }
  return cpu_freq;
}

void enable_core_timer() {
  uint64_t one = 1;
  asm volatile("msr cntp_ctl_el0, %0" :: "r" (one));
  set_abs_timer(0x7FFFFFFFFFFFFFFFl);
  *CORE0_TIMER_IRQ_CTRL = 2;
}

uint64_t get_current_time() {
  volatile uint64_t cur;
  asm volatile("mrs %0, cntpct_el0" : "=r" (cur));
  return cur;
}

void set_relative_timer(uint32_t tv_cycle) {
  volatile register uint64_t tv_cycle_64 = tv_cycle;
  asm volatile("msr cntp_tval_el0, %0" :: "r" (tv_cycle_64));
}

void set_abs_timer(uint64_t cval) {
  asm volatile("msr cntp_cval_el0, %0" :: "r" (cval));
}

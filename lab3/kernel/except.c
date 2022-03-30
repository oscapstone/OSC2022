#include "mini_uart.h"
#include "except.h"
#include "timer.h"
#include <stdint.h>

#define CORE0_IRQ_SOURCE ((volatile unsigned int *)(0x40000060))
#define GPU_PENDING1 ((volatile unsigned int *)(0x3F00B204))

#define RECV_BUFFER_SIZE 1024
#define SEND_BUFFER_SIZE 1024

char recv_buffer[RECV_BUFFER_SIZE];
unsigned int recv_head;
unsigned int recv_tail;
char send_buffer[SEND_BUFFER_SIZE];
unsigned int send_head;
unsigned int send_tail;

void async_print(const char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    if (str[i] == '\n') {
      send_buffer[send_tail++] = '\r';
      if (send_tail >= SEND_BUFFER_SIZE) send_tail -= SEND_BUFFER_SIZE;
    }
    send_buffer[send_tail++] = str[i];
    if (send_tail >= SEND_BUFFER_SIZE) send_tail -= SEND_BUFFER_SIZE;
  }
  if (send_tail != send_head) {
    *AUX_MU_IER = 3;
  }
}

unsigned int async_read(char *str, unsigned int size) {
  int i;
  for (i = 0; i < size-1; i++) {
    while (recv_head == recv_tail) asm volatile("nop");
    if (recv_buffer[recv_head] == '\r') {
      recv_head++;
      if (recv_head >= RECV_BUFFER_SIZE) recv_head -= RECV_BUFFER_SIZE;
      break;
    } else {
      str[i] = recv_buffer[recv_head];
    }
    recv_head++;
    if (recv_head >= RECV_BUFFER_SIZE) recv_head -= RECV_BUFFER_SIZE;
  }
  str[i] = '\0';
  return i;
}

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
  uint64_t freq = get_cpu_freq();
  uint64_t cur = get_current_time();
  print("cpu freq: ");
  print_num(freq);
  print("\nLive time: ");
  print_num(cur/freq);
  print_char('\n');
  reset_timer(2*freq);
}


void uart_recv_to_buf() {
  if (recv_tail >= RECV_BUFFER_SIZE) {
    recv_tail -= RECV_BUFFER_SIZE;
  }
  char ch = mini_uart_recv();
  mini_uart_send(ch);
  if (ch == '\r') mini_uart_send('\n');
  recv_buffer[recv_tail++] = ch;
}


void uart_send_from_buf() {
  if (send_head == send_tail) {
    print("transmit interrupt should be disabled\n");
    *AUX_MU_IER = 1;
  }
  mini_uart_send(send_buffer[send_head++]);
  if (send_head >= SEND_BUFFER_SIZE) {
    send_head -= SEND_BUFFER_SIZE;
  }
  if (send_head == send_tail) {
    volatile register unsigned int ier = *AUX_MU_IER;
    ier &= (~2); // bit 1 enables transmit interrupt, now we want to disable it
    *AUX_MU_IER = ier;
    return;
  }
}

void gpu_interrupt_handler() {
  if ((*GPU_PENDING1)&(1<<29)) {
    // uart interrupt (actually aux interrupt)
    volatile unsigned int iir = *AUX_MU_IIR;
    if ((iir & 1) == 0) {
      if (iir & 2) {
        uart_send_from_buf();
        // print("Transmit holding register empty\n");
      } else if (iir & 4) {
        // print("recv interrupt\n");
        uart_recv_to_buf();
      }
    } else {
      print("Aux Interrupt Error ");
      print_hex(iir);
      print_char('\n');
    }
  }
}

void c_irq_handler() {
  volatile uint64_t irq_src = *CORE0_IRQ_SOURCE;
  if (irq_src & (1<<1)) {
    // bit 1 indicates cntpnsirq interrupt
    core_timer_handler();
  } else if (irq_src & (1<<8)) {
    gpu_interrupt_handler();
  } else {
    print("Invalid interrupt source ");
    print_hex(irq_src);
    print_char('\n');
  }
}

void el1h_c_irq_handler() {
  volatile uint64_t irq_src = *CORE0_IRQ_SOURCE;
  if (irq_src & (1<<1)) {
    print("receive core timer interrupt\n");
    // bit 1 indicates cntpnsirq interrupt
    handle_timer_interrupt();
  } else if (irq_src & (1<<8)) {
    print("receive gpu interrupt\n");
  } else {
    print("Invalid interrupt source ");
    print_hex(irq_src);
    print_char('\n');
  }
}

void c_invalid_exception() {
  print("Invalid exception\n");
}

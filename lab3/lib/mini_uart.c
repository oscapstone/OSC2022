#include "mini_uart.h"

#define AUX_ENABLE  ((volatile unsigned int *)(0x3F215004))
#define AUX_MU_IO   ((volatile unsigned int *)(0x3F215040))
#define AUX_MU_IER  ((volatile unsigned int *)(0x3F215044))
#define AUX_MU_IIR  ((volatile unsigned int *)(0x3F215048))
#define AUX_MU_LCR  ((volatile unsigned int *)(0x3F21504C))
#define AUX_MU_MCR  ((volatile unsigned int *)(0x3F215050))
#define AUX_MU_LSR  ((volatile unsigned int *)(0x3F215054))
#define AUX_MU_CNTL ((volatile unsigned int *)(0x3F215060))
#define AUX_MU_BAUD ((volatile unsigned int *)(0x3F215068))

#define GPFSEL1 ((volatile unsigned int *)(0x3F200004))
#define GPPUD ((volatile unsigned int *)(0x3F200094))
#define GPPUDCLK0 ((volatile unsigned int *)(0x3F200098))

void mini_uart_init() {
  register volatile unsigned int rsel = *GPFSEL1;
  register volatile unsigned int rclk0;
  
  rsel &= 0xFFFC0FFF;
  rsel |= ((2<<12)|(2<<15));
  *GPFSEL1 = rsel;
  *GPPUD = 0;
  
  int waitcycles = 150;
  while (waitcycles--)  asm volatile("nop");
  rclk0 = ((1<<14)|(1<<15));
  *GPPUDCLK0 = rclk0;
  waitcycles = 150;
  while (waitcycles--) asm volatile("nop");
    
  *AUX_ENABLE |= 1;
  *AUX_MU_CNTL = 0;
  *AUX_MU_IER = 0;
  *AUX_MU_LCR = 3;
  *AUX_MU_MCR = 0;
  *AUX_MU_BAUD = 270;
  *AUX_MU_IIR = 6;
  *AUX_MU_CNTL = 3;

}

void mini_uart_send(char c) {
  while (1) {
    if ((*AUX_MU_LSR)&0x20) break;
  }
  *AUX_MU_IO = c;
}

char mini_uart_recv() {
  while (1) {
    if ((*AUX_MU_LSR)&0x01) break;
  }
  return (*AUX_MU_IO)&0xFF;
}


void print_char(const char c) {
  if (c == '\n') mini_uart_send('\r');
  mini_uart_send(c);
}

void print(const char *str) {
  while (*str) {
    print_char(*str++);
  }
}

void print_num(int num) {
  if (num == 0) {
    print_char('0');
    return;
  }
  if (num < 0) {
    print_char('-');
    num = -num;
  }
  char buf[10];
  int len = 0;
  while (num > 0) {
    buf[len++] = (char)(num%10)+'0';
    num /= 10;
  }
  for (int i = len-1; i >= 0; i--) {
    print_char(buf[i]);
  }
}

void print_hex(unsigned int num) {
  print("0x");
  int h = 28;
  while (h >= 0) {
    char ch = (num >> h) & 0xF;
    if (ch >= 10) ch += 'A' - 10;
    else ch += '0';
    print_char(ch);
    h -= 4;
  }
}

void print_hexl(unsigned long num) {
  print("0x");
  int h = 60;
  while (h >= 0) {
    char ch = (num >> h) & 0xF;
    if (ch >= 10) ch += 'A' - 10;
    else ch += '0';
    print_char(ch);
    h -= 4;
  }
}



int read(char *buf, int len) {
  char c;
  int i;
  for (i = 0; i < len; i++) {
    c = mini_uart_recv();
    if (c == 127) { i--; continue; }
    print_char(c);
    // print_num((int)c);
    if (c == '\r') {
      c = '\n';
      print_char('\n');
      break;
    }
    buf[i] = c;
  }
  buf[i] = '\0';
  return i;
}

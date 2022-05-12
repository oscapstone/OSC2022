/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "gpio.h"
#include "utils.h"
#include "uart.h"
/* Auxilary mini UART registers */
#define AUX_ENABLE      ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE+0x00215068))
/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    register unsigned int r;
    
    /* initialize UART */
    *AUX_ENABLE |=1;       // enable UART1, AUX mini uart
    *AUX_MU_CNTL = 0;
    *AUX_MU_LCR = 3;       // 8 bits
    *AUX_MU_MCR = 0;
    *AUX_MU_IER = 1;
    // comment this line to avoid weird character
    // *AUX_MU_IIR = 0xc6;    // disable interrupts
    *AUX_MU_BAUD = 270;    // 115200 baud
    /* map UART1 to GPIO pins */
    r=*GPFSEL1;
    r&=~((7<<12)|(7<<15)); // gpio14, gpio15
    r|=(2<<12)|(2<<15);    // alt5
    *GPFSEL1 = r;
    *GPPUD = 0;            // enable pins 14 and 15
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = (1<<14)|(1<<15);
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;        // flush GPIO setup
    *AUX_MU_CNTL = 3;      // enable Tx, Rx

    read_buf_start = read_buf_end = 0;
    write_buf_start = write_buf_end = 0;
    enable_uart_interrupt();
}

/**
 * Send a character
 */
void uart_send(unsigned int c) {
    /* wait until we can send */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
    /* write the character to the buffer */
    *AUX_MU_IO=c;
}

/**
 * Receive a character
 */
char uart_getc() {
    char r;
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x01));
    /* read it and return */
    r=(char)(*AUX_MU_IO);
    /* convert carrige return to newline */
    return r=='\r'?'\n':r;
}

uint32_t uart_gets(char *buf, uint32_t size) {
  for (int i = 0; i < size; ++i) {
    buf[i] = uart_getc();
    // uart_send(buf[i]);
    if (buf[i] == '\n' || buf[i] == '\r') {
      // uart_send('\r');
      // buf[i] = '\0';
      return i;
    }
  }
  return size;
}

/**
 * Display a string
 */
void uart_puts(char *s) {
    while(*s) {
        /* convert newline to carrige return + newline */
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

uint32_t uart_write(char *s, uint32_t size) {
    for(int i = 0 ; i<size ;i++){
      uart_send(s[i]);
      if(s[i]=='\n'){
        uart_send('\r');
        return i;
      }
    }
    return size;
}

/**
 * Display a binary value in hexadecimal
 */
void uart_hex(unsigned int d) {
    unsigned int n;
    int c;
    for(c=28;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_send(n);
    }
}

void uart_int(int x) {
  if (x < 0) {
    uart_send('-');
    x = -x;
  }
  if (x >= 10) uart_int(x / 10);
  uart_send(x % 10 + '0');
}

void enable_uart_interrupt() { *ENABLE_IRQS_1 = AUX_IRQ; }

void disable_uart_interrupt() { *DISABLE_IRQS_1 = AUX_IRQ; }

void assert_transmit_interrupt() { *AUX_MU_IER |= 0x2; }

void clear_transmit_interrupt() { *AUX_MU_IER &= ~(0x2); }

void uart_handler() {
  disable_uart_interrupt();
  int is_read = (*AUX_MU_IIR & 0x4);
  int is_write = (*AUX_MU_IIR & 0x2);

  if (is_read) {
    // uart_puts("===== is_read =====\n");

    char c = (char)(*AUX_MU_IO);
    read_buf[read_buf_end++] = c;
    if (read_buf_end == UART_BUFFER_SIZE) read_buf_end = 0;
  } else if (is_write) {
    // uart_puts("===== is_write =====\n");

    while (*AUX_MU_LSR & 0x20) {
      if (write_buf_start == write_buf_end) {
        clear_transmit_interrupt();
        break;
      }
      char c = write_buf[write_buf_start++];
      *AUX_MU_IO = c;
      if (write_buf_start == UART_BUFFER_SIZE) write_buf_start = 0;
    }
  }
  enable_uart_interrupt();
}

char uart_async_getc() {
  // wait until there are new data
    // uart_puts("===== uart getc =====\n");
    while (read_buf_start == read_buf_end) {
        // uart_puts("===== read_buf_start == read_buf_end =====\n");
        asm volatile("nop");
    }
    char c = read_buf[read_buf_start++];
    if (read_buf_start == UART_BUFFER_SIZE) read_buf_start = 0;
    // '\r' => '\n'
    return c == '\r' ? '\n' : c;
}

void uart_async_puts(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] == '\n') write_buf[write_buf_end++] = '\r';
        write_buf[write_buf_end++] = str[i];
        if (write_buf_end == UART_BUFFER_SIZE) write_buf_end = 0;
    }
    assert_transmit_interrupt();
}

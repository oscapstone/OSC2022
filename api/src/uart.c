#include "gpio.h"
#include "uart.h"
#include <stdarg.h>
#include <stdio.h>

/* Auxilary mini UART registers */
#define AUX_ENABLE    ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO     ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER    ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR    ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR    ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR    ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR    ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR    ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL   ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT   ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD   ((volatile unsigned int*)(MMIO_BASE+0x00215068))

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
  register unsigned int r;

  // Initialize UART
  *AUX_ENABLE |= 1;    // enable UART1, AUX mini uart
  *AUX_MU_CNTL = 0;
  *AUX_MU_LCR  = 3;    // 8 bits
  *AUX_MU_MCR  = 0;
  *AUX_MU_IER  = 0;
  *AUX_MU_IIR  = 0xC6;   // disable interrupts
  *AUX_MU_BAUD = 270;  // 115200 baud

  // Map UART1 to GPIO pins
  r  = *GPFSEL1;
  r &= ~((7<<12) | (7<<15));  // clear GPIO14, 15's state. and leave other's alone
  r |=   (2<<12) | (2<<15);   // 2 is alt5, 12~14th bit for GPIO14, 15th~17 for GPIO15
  *GPFSEL1 = r;

  // Set GPIO14, 15 to no pull up/down. Follow the steps of p.101, BCM2837 datasheet
  *GPPUD = 0x00;          // step 1. no pull up, no pull down
  WAIT_TICKS(r, 150);       // step 2. wait 150 ticks for signal to set-up
  *GPPUDCLK0 = (1<<14) | (1<<15); // step 3. select clock for GPIO14, 15
  WAIT_TICKS(r, 150);       // step 4. wait 150 ticks for signal to hold
  *GPPUD = 0x00;          // step 5.
  *GPPUDCLK0 = 0;         // step 6. flush GPIO setup
  *AUX_MU_CNTL = 3;       // enable Tx, Rx
}

/* Wrapping snprintf and HAL_UART_Transmit, just use this as printf
  Note that this function does no buffering like printf.
  The maximum characters to print at once is 128,
  you can change buf size for that.
*/
void uart_printf(const char *fmt, ...){
  int len = 0;
  char buf[128];
  va_list args;
  va_start(args, fmt);
  len = vsnprintf(buf, sizeof(buf), fmt, args);
  if(len > 0)
    uart_puts(buf);
  va_end(args);
}

/**
 * Send a character
 */
void uart_send(unsigned int c) {
  // wait until we can send
  while(!(*AUX_MU_LSR&0x20));
  // write the character to the buffer
  *AUX_MU_IO = c;
}

/**
 * Receive a character
 */
char uart_getc() {
  char r;
  // wait until something is in the buffer
  while(!(*AUX_MU_LSR&0x01));

  // read it and return
  r = (char)(*AUX_MU_IO);
  
  // convert carrige return to newline
  r = (r=='\r' ? '\n' : r);
  return r;
}

/**
 * Display a string
 */
void uart_puts(char *s) {
  while(*s) {
    // convert newline to carrige return + newline
    if(*s=='\n')
      uart_send('\r');

    uart_send(*s++);
  }
}

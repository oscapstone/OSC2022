#ifndef __UART__H__
#define __UART__H__
#include "gpio.h"
#include "exception.h"
#include "stddef.h"

/* Auxilary  UART registers */
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

/* Asynchronous Read and Write */
#define READ_BUF_SIZE          (1024)
#define WRITE_BUF_SIZE         (1024)
// extern char read_buffer[READ_BUF_SIZE];
// extern char write_buffer[WRITE_BUF_SIZE];

// extern unsigned int read_head;
// extern unsigned int write_head;
// extern unsigned int read_tail;
// extern unsigned int write_tail;
void uart_irq_handler ();

void uart_init();
void uart_send(unsigned int c);
char uart_getc();
void uart_puts(char *s);
void uart_hex(unsigned int d);
void uart_getline(char *input);
void uart_put_int();
// void uart_put (char c);

void asyn_uart_init();
// void set_uart_rx_int(bool enable);
// void set_uart_tx_int(bool enable);
char asyn_uart_get();
void asyn_uart_puts (char *s);
void enable_uart_interrupt();
void disable_uart_interrupt();
void enable_write_interrupt();
void disable_write_interrupt();
void enable_read_interrupt();
void disable_read_interrupt();

char async_uart_getc();
void asyn_uart_put (char c);
#endif
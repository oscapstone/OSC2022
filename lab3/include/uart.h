#ifndef UART_H
#define UART_H

#define MAX_BUF_SIZE 0x100

static char* uart_tx_buffer[MAX_BUF_SIZE]={};
static unsigned int uart_tx_buffer_widx = 0;
static unsigned int uart_tx_buffer_ridx = 0;
static char* uart_rx_buffer[MAX_BUF_SIZE]={};
static unsigned int uart_rx_buffer_widx = 0;
static unsigned int uart_rx_buffer_ridx = 0;

void uart_init();
void uart_putc(char c);
char uart_getc();
int uart_puts(char *s);
char* uart_gets(char *buf);
int uart_printf(char *fmt, ...);
void uart_hex(unsigned int d);
void disable_uart();
void enable_mini_uart_interrupt();
void uart_interrupt_handler();

#endif

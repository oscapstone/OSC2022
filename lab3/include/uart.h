#ifndef UART_H
#define UART_H

#define MAX_BUF_SIZE 0x100

void uart_init();
void uart_putc(char c);
char uart_getc();
int uart_puts(char *s);
int uart_async_puts(char *s);
char* uart_gets(char *buf);
char* uart_async_gets(char *buf);
int uart_printf(char *fmt, ...);
int uart_async_printf(char *fmt, ...);
void disable_uart();
void uart_interrupt_handler();
void enable_mini_uart_interrupt();
void enable_mini_uart_w_interrupt();
void enable_mini_uart_r_interrupt();
void disable_mini_uart_interrupt();
void disable_mini_uart_w_interrupt();
void disable_mini_uart_r_interrupt();

char uart_async_getc();
void uart_async_putc(char c);

#endif

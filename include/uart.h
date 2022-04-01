#ifndef UART
#define UART

#define MAX_BUF_SIZE 1024

void uart_init();
void uart_enable();
void uart_disable();
char uart_getb();
char uart_getc();
void uart_putc(unsigned int c);
void uart_puts(char *s);
void uart_hex(unsigned int d);
void uart_num(unsigned int d);
unsigned int uart_getn();
void uart_newline();
void uart_dem();
void uart_prefix();
char* uart_img_receiver(char* address);

void uart_interrupt_r_handler();
void uart_interrupt_w_handler();
void uart_async_puts(char *s);
void uart_async_newline();
void uart_async_dem();
void uart_async_prefix();
void uart_async_hex(unsigned int d);
void uart_async_num(unsigned int d);
unsigned int uart_async_putc(char c);
char uart_async_getc();
void enable_uart_interrupt();
void enable_uart_r_interrupt();
void enable_uart_w_interrupt();
void disable_uart_interrupt();
void disable_uart_r_interrupt();
void disable_uart_w_interrupt();
#endif

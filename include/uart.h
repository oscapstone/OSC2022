#ifndef UART
#define UART

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

#endif

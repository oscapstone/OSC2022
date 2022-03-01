void uart_init();
void uart_enable();
void uart_disable();
char uart_getc();
void uart_putc(unsigned int c);
void uart_puts(char *s);
void uart_hex(unsigned int d);
void uart_num(unsigned int d);
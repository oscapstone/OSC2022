void uart_init();
void uart_send(unsigned int c);
char uart_getc();
void uart_puts(char *s);
char uart_read();
void uart_write(unsigned int c);
void uart_printf(char* fmt, ...);
void uart_flush();
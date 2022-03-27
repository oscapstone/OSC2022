void enable_uart_interrupt();
void disable_uart_interrupt();
void uart_init();
void uart_send(unsigned int c);
char uart_getc();
void uart_puts(char *s);
void uart_hex(unsigned int d);
unsigned int printf(char* fmt,...);

void handle_uart_irq();

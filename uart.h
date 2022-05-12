#define ASYNC_UART
void enable_uart_interrupt();
void disable_uart_interrupt();
void enable_transmit_interrupt();
void uart_init();
void uart_send(unsigned int c);
char uart_getc();
void uart_puts(char *s);
int uart_gets(char* s,int size,int display);
void uart_hex(unsigned int d);
void printf(char* fmt,...);

void handle_uart_irq();
char async_uart_getc();
void async_uart_send(const char *str, unsigned long size);
void async_uart_puts(char *str);
void test_async_write();
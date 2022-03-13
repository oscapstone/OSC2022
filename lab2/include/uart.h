void	uart_init();
void	uart_flush();
void	uart_send(unsigned int c);
char	uart_getc();
void 	uart_hex(unsigned int d);
void 	uart_printf(char *fmt, ...);
int 	uart_get_int();

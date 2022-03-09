#ifndef _UART_H_
#define _UART_H_


void UART_init();
char UART_read();
void UART_write(unsigned int c);
void UART_put(char *s);
void UART_hex(unsigned int c);

#endif


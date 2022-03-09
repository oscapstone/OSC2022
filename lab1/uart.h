#ifndef _UART_H
#define _UART_H

#define BUFFER_SIZE     0x100

void  uart_init();
void  uart_send(unsigned int c);
char  uart_getc();
char* uart_gets(char* buffer);
void  uart_puts(char *s);
void  uart_hex(unsigned int d);
int   uart_int(int d);
void  uart_printf();

#endif
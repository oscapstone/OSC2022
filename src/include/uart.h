#include <stddef.h>

#ifndef _DEF_UART
#define _DEF_UART

void uart_init();
size_t uart_read(char* buf, size_t len);
size_t uart_write(char* buf, size_t len);
void uart_print(char* buf);
void uart_puts(char* buf);
size_t uart_gets(char* buf);

#endif
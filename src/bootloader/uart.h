#include <stddef.h>
#include <stdint.h>

#ifndef _DEF_UART
#define _DEF_UART

void uart_init();
size_t uart_write(const char* buf, size_t len);
void uart_print(const char* buf);
void uart_puts(const char* buf);
size_t uart_gets(char* buf);
void uart_putshex(uint64_t num);
void uart_print_hex(uint64_t num);
void uart_interrupt_handler();
size_t uart_read(char* buf, size_t len);

#endif
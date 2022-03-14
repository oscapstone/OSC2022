#ifndef UART_H
#define UART_H


#include "bcm2357.h"

#ifndef WITH_STDLIB
#include "type.h"
#else
#include <stdint.h>
#endif

#include "uart_config.h"


void init_uart(uint32_t baud_rate);
void uart_send(char);
char uart_recv();
void uart_write(char*);
void uart_hex(uint32_t);
char hex2a(uint8_t);
void uart_writeb(char*, uint32_t);

void uart_putc(char c);
char uart_getc();



void pll0_uart_init();
void pll0_uart_putc(unsigned int c);
void pll0_uart_send(unsigned int c);
char pll0_uart_getc();
void pll0_uart_write(char*);
void pll0_uart_hex(uint32_t num);
void pll0_uart_flush();
void pll0_uart_writeb(char*, uint32_t);

#endif
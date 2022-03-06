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

void uart_putc(char c);
char uart_getc();



#endif
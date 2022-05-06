#ifndef _MINI_UART_H_
#define _MINI_UART_H_

#include "types.h"

#define SYSTEM_CLOCK_RATE 25000000
#define BAUD_RATE 115200
#define BAUD_RATE_REG SYSTEM_CLOCK_RATE / (8 * BAUD_RATE) - 1

extern void mini_uart_init(void);
extern uint8_t mini_uart_read(void);
extern void mini_uart_write(uint8_t);
extern ssize_t write_bytes(uint8_t *, size_t);
extern void write_str(char *);



#endif

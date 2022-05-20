#ifndef _MINI_UART_H_
#define _MINI_UART_H_

#include "types.h"

#define SYSTEM_CLOCK_RATE (250000000)
#define BAUD_RATE 115200
#define BAUD_RATE_REG (SYSTEM_CLOCK_RATE / (8 * BAUD_RATE) - 1)
#define TX 1 
#define RX 2
extern void mini_uart_init();
extern uint8_t mini_uart_read(void);
extern void mini_uart_write(uint8_t);
extern ssize_t write_bytes(uint8_t *, size_t);
extern void write_str(char *);
extern void write_hex(uint64_t);
extern void delay_cycles(uint64_t);
extern void disable_mini_uart_irq(uint32_t);
extern void enable_mini_uart_irq(uint32_t);
extern void mini_uart_irq_init();
extern void mini_uart_irq_read();
extern size_t mini_uart_get_rx_len();
extern uint8_t mini_uart_aio_read(void);
extern void mini_uart_irq_write();
extern size_t mini_uart_get_tx_len();
extern void mini_uart_aio_write(uint8_t);



#endif

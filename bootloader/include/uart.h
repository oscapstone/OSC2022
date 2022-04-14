#ifndef UART_H
#define UART_H
#include "gpio.h"

#define MAX_BUF_SIZE 0x100

#define AUX_BASE            (MMIO_BASE + 0x00215000)

#define AUX_IRQ             ((volatile unsigned int*)(AUX_BASE + 0x00))
#define AUX_ENABLES         ((volatile unsigned int*)(AUX_BASE + 0x04))
#define AUX_MU_IO_REG       ((volatile unsigned int*)(AUX_BASE + 0x40))
#define AUX_MU_IER_REG      ((volatile unsigned int*)(AUX_BASE + 0x44))
#define AUX_MU_IIR_REG      ((volatile unsigned int*)(AUX_BASE + 0x48))
#define AUX_MU_LCR_REG      ((volatile unsigned int*)(AUX_BASE + 0x4c))
#define AUX_MU_MCR_REG      ((volatile unsigned int*)(AUX_BASE + 0x50))
#define AUX_MU_LSR_REG      ((volatile unsigned int*)(AUX_BASE + 0x54))
#define AUX_MU_MSR_REG      ((volatile unsigned int*)(AUX_BASE + 0x58))
#define AUX_MU_SCRATCH      ((volatile unsigned int*)(AUX_BASE + 0x5c))
#define AUX_MU_CNTL_REG     ((volatile unsigned int*)(AUX_BASE + 0x60))
#define AUX_MU_STAT_REG     ((volatile unsigned int*)(AUX_BASE + 0x64))
#define AUX_MU_BAUD_REG     ((volatile unsigned int*)(AUX_BASE + 0x68))
#define AUX_SPI0_CNTL0_REG  ((volatile unsigned int*)(AUX_BASE + 0x80))
#define AUX_SPI0_CNTL1_REG  ((volatile unsigned int*)(AUX_BASE + 0x84))
#define AUX_SPI0_STAT_REG   ((volatile unsigned int*)(AUX_BASE + 0x88))
#define AUX_SPI0_IO_REG     ((volatile unsigned int*)(AUX_BASE + 0x90))
#define AUX_SPI0_PEEK_REG   ((volatile unsigned int*)(AUX_BASE + 0x94))
#define AUX_SPI1_CNTL0_REG  ((volatile unsigned int*)(AUX_BASE + 0xc0))
#define AUX_SPI1_CNTL1_REG  ((volatile unsigned int*)(AUX_BASE + 0xc4))
#define AUX_SPI1_STAT_REG   ((volatile unsigned int*)(AUX_BASE + 0xc8))
#define AUX_SPI1_IO_REG     ((volatile unsigned int*)(AUX_BASE + 0xd0))
#define AUX_SPI1_PEEK_REG   ((volatile unsigned int*)(AUX_BASE + 0xd4))

void uart_init();
void uart_write_char(char c);
void uart_write_string(char* str);
void uart_write_hex(unsigned int d);
void uart_read(char* buf, unsigned int size);
void nop_delay(unsigned int t);

#endif
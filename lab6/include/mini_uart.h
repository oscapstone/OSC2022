#ifndef MINI_UART_H
#define MINI_UART_H

#include <stdint.h>
#include "mmu.h"

#define MMIO_BASE_ADDR (0x3F000000 + VA_START)
#define AUX_ENABLE  ((volatile unsigned int *)(0x3F215004 + VA_START))
#define AUX_MU_IO   ((volatile unsigned int *)(0x3F215040 + VA_START))
#define AUX_MU_IER  ((volatile unsigned int *)(0x3F215044 + VA_START))
#define AUX_MU_IIR  ((volatile unsigned int *)(0x3F215048 + VA_START))
#define AUX_MU_LCR  ((volatile unsigned int *)(0x3F21504C + VA_START))
#define AUX_MU_MCR  ((volatile unsigned int *)(0x3F215050 + VA_START))
#define AUX_MU_LSR  ((volatile unsigned int *)(0x3F215054 + VA_START))
#define AUX_MU_CNTL ((volatile unsigned int *)(0x3F215060 + VA_START))
#define AUX_MU_BAUD ((volatile unsigned int *)(0x3F215068 + VA_START))


#define GPFSEL1 ((volatile unsigned int *)(0x3F200004 + VA_START))
#define GPPUD ((volatile unsigned int *)(0x3F200094 + VA_START))
#define GPPUDCLK0 ((volatile unsigned int *)(0x3F200098 + VA_START))

#define ENABLE_IRQs1 ((volatile unsigned int *)(0x3F00B210 + VA_START))

void mini_uart_init();
void mini_uart_send(char data);
char mini_uart_recv();

void print_char(const char c);
void print(const char *str);
void print_num(int num);
void print_hex(unsigned int num);
void print_hexl(unsigned long num);
int readline(char *buf, int len);

uint32_t syscall_uart_read(char buf[], uint32_t size);
uint32_t syscall_uart_write(const char buf[], uint32_t size);

#endif

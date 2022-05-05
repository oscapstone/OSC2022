#ifndef MINI_UART_H
#define MINI_UART_H

#include <stdint.h>

#define AUX_ENABLE  ((volatile unsigned int *)(0x3F215004))
#define AUX_MU_IO   ((volatile unsigned int *)(0x3F215040))
#define AUX_MU_IER  ((volatile unsigned int *)(0x3F215044))
#define AUX_MU_IIR  ((volatile unsigned int *)(0x3F215048))
#define AUX_MU_LCR  ((volatile unsigned int *)(0x3F21504C))
#define AUX_MU_MCR  ((volatile unsigned int *)(0x3F215050))
#define AUX_MU_LSR  ((volatile unsigned int *)(0x3F215054))
#define AUX_MU_CNTL ((volatile unsigned int *)(0x3F215060))
#define AUX_MU_BAUD ((volatile unsigned int *)(0x3F215068))


#define GPFSEL1 ((volatile unsigned int *)(0x3F200004))
#define GPPUD ((volatile unsigned int *)(0x3F200094))
#define GPPUDCLK0 ((volatile unsigned int *)(0x3F200098))

#define ENABLE_IRQs1 ((volatile unsigned int *)(0x3F00B210))

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

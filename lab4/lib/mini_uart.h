#ifndef MINI_UART_H
#define MINI_UART_H

#define AUX_ENABLE  ((volatile unsigned int *)(0x3F215004))
#define AUX_MU_IO   ((volatile unsigned int *)(0x3F215040))
#define AUX_MU_IER  ((volatile unsigned int *)(0x3F215044))
#define AUX_MU_IIR  ((volatile unsigned int *)(0x3F215048))
#define AUX_MU_LCR  ((volatile unsigned int *)(0x3F21504C))
#define AUX_MU_MCR  ((volatile unsigned int *)(0x3F215050))
#define AUX_MU_LSR  ((volatile unsigned int *)(0x3F215054))
#define AUX_MU_CNTL ((volatile unsigned int *)(0x3F215060))
#define AUX_MU_BAUD ((volatile unsigned int *)(0x3F215068))

void mini_uart_init();
void mini_uart_send(char data);
char mini_uart_recv();

void print_char(const char c);
void print(const char *str);
void print_num(int num);
void print_hex(unsigned int num);
void print_hexl(unsigned long num);
int read(char *buf, int len);

#endif

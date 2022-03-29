#ifndef MINI_UART_H
#define MINI_UART_H

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

#ifndef __UART__H__
#define __UART__H__
#define AUX_MU_LSR_TRANS_IDEL  (1 << 6)
#define AUX_MU_LSR_DATA_READY  (1)

void uart_init();
void uart_flush();
char uart_get();
void uart_putc(char c);
void uart_puth(unsigned int d);
void uart_puts(char *s);
#endif
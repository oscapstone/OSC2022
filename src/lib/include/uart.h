#ifndef __UART__H__
#define __UART__H__
#define AUX_MU_LSR_TRANS_IDEL  (1 << 6)
#define AUX_MU_LSR_DATA_READY  (1)

void uart_init();
void uart_flush();
void uart_put(char c);
char uart_get();
#endif
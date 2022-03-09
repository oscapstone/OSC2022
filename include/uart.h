#ifndef UART_H_
#define UART_H_
#include <gpio.h>

#define AUX_BASE        (MMIO_BASE + 0x00215000) //MMIO base address + AUX offset
#define AUX_ENABLE      ((volatile unsigned int*)(AUX_BASE+0x4))
#define AUX_MU_IO       ((volatile unsigned int*)(AUX_BASE+0x40))
#define AUX_MU_IER      ((volatile unsigned int*)(AUX_BASE+0x44))
#define AUX_MU_IIR      ((volatile unsigned int*)(AUX_BASE+0x48))
#define AUX_MU_LCR      ((volatile unsigned int*)(AUX_BASE+0x4C))
#define AUX_MU_MCR      ((volatile unsigned int*)(AUX_BASE+0x50))
#define AUX_MU_LSR      ((volatile unsigned int*)(AUX_BASE+0x54))
#define AUX_MU_MSR      ((volatile unsigned int*)(AUX_BASE+0x58))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(AUX_BASE+0x5C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(AUX_BASE+0x60))
#define AUX_MU_STAT     ((volatile unsigned int*)(AUX_BASE+0x64))
#define AUX_MU_BAUD     ((volatile unsigned int*)(AUX_BASE+0x68))

/* Display a char */
void uart_init();

/* Send a character */
void uart_send(unsigned int);

/* Receive a character */
char uart_getc();

/* Display a string */
void uart_puts(char*);


#endif
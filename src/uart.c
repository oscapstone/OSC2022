#include "uart.h"
#include "gpio.h"

void uart_init()
{
    register unsigned int r;

    /* initialize UART */
    *AUX_ENABLE |= 1;      // enable UART1, AUX mini uart
    *AUX_MU_CNTL = 0;      // disable transmitter and receiver
    *AUX_MU_LCR = 3;       // set the data size to 8 bits
    *AUX_MU_MCR = 0;       // don't need auto flow control
    *AUX_MU_IER = 0;       // disable interrupt
    *AUX_MU_IIR = 0xc6;    // no FIFO
    *AUX_MU_BAUD = 270;    // set baud rate to 115200

    /* map UART1 to GPIO pins */
    r = *GPFSEL1;
    r &= ~((7 << 12) | (7 << 15)); // clear GPIO14, GPIO15, each GPIO pin has 3-bit
    r |= (2 << 12) | (2 << 15);    // set GPIO14, GPIO15 to ALT5
    *GPFSEL1 = r;
    *GPPUD = 0;            // enable pins 14 and 15
    r = 150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;        // flush GPIO setup
    *AUX_MU_CNTL = 3;      // enable Tx, Rx
}

void uart_putc(unsigned int c)
{
    // Send a character
    /* wait until we can send */
    do {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR & 0x20));
    /* write the character to the buffer */
    *AUX_MU_IO = c;
}

char uart_getc()
{
    char c;
    /* wait until something is in the buffer */
    do {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR & 0x01));
    /* read it and return */
    c = (char)(*AUX_MU_IO);
    /* convert carrige return to newline */
    return (c == '\r')? '\n' : c;
}

/**
 * Display a string
 */
void uart_puts(char *s)
{
    while (*s) {
        /* convert newline to carrige return + newline */
        if (*s == '\n')
            uart_putc('\r');
        uart_putc(*s++);
    }
}
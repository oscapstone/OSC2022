#include "uart.h"
#include "gpio.h"
#include "aux.h"

void uart_init () {
    unsigned int d;
    /* Set mini uart enable */
    d = mmio_get(AUX_ENABLES);
    d = d | 1;
    mmio_put(AUX_ENABLES, d);
    /* Disable transmitter and receiver during configuration */
    mmio_put(AUX_MU_CNTL_REG, 0);
    /* Disable interrupt */
    mmio_put(AUX_MU_IER_REG, 0);
    /* Set the data size to 8 bit */
    mmio_put(AUX_MU_LCR_REG, 3);
    /* Disable auto flow control */
    mmio_put(AUX_MU_MCR_REG, 0);
    /* Set baud rate to 115200 */
    mmio_put(AUX_MU_BAUD_REG, 270);
    /* No FIFO */
    mmio_put(AUX_MU_IIR_REG, 6);

    /* Get previous status of GPFSEL1 */
    d = mmio_get(GPFSEL1); 
    /* Clear GPIO14, GPIO15 setting */
    d = d & ~( (7 << GPIO14_GPFSEL_OFFSET) | (7 << GPIO15_GPFSEL_OFFSET) ); 
    /* Set GPIO14, GPIO15 to ALT5 */
    d = d | (2 << GPIO14_GPFSEL_OFFSET) | (2 << GPIO15_GPFSEL_OFFSET);
    mmio_put(GPFSEL1, d);
    /* Disable GPIO PULL UP/DOWN */
    mmio_put(GPPUD, 0);
    /* Wait 150 cycles (required set-up time) */
    for (int i = 0; i < 150; i++) asm volatile ("nop");
    /* Enable clock for setting GPIO14, GPIO15*/
    d = ( (1 << GPIO14_GPPUDCLK_OFFSET) | (1 << GPIO15_GPPUDCLK_OFFSET) );
    mmio_put(GPPUDCLK0, d);
    /* Wait 150 cycles (required set-up time) */
    for (int i = 0; i < 150; i++) asm volatile ("nop");
    /* Remove clock */
    mmio_put(GPPUDCLK0, 0);
    /* Enable transmitter and receiver */
    mmio_put(AUX_MU_CNTL_REG, 3);

    return;
}

void uart_flush () {
    while (mmio_get(AUX_MU_LSR_REG) & AUX_MU_LSR_DATA_READY) mmio_get(AUX_MU_IO_REG);
}

void uart_put (char c) {
    /* Wait for transmitter empty */
    while ((mmio_get(AUX_MU_LSR_REG) & AUX_MU_LSR_TRANS_IDEL) == 0) asm volatile ("nop");
    /* Put data */
    mmio_put(AUX_MU_IO_REG, c);

    return;
}

char uart_get () {
    char c;
    /* Wait for data ready */
    while ((mmio_get(AUX_MU_LSR_REG) & AUX_MU_LSR_DATA_READY) == 0) asm volatile ("nop");
    c = mmio_get(AUX_MU_IO_REG);
    if (c == '\r') c = '\n';

    return c;
}
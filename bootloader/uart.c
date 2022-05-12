#include "uart.h"

void uart_init() {
    register unsigned int r;

    /* initialize UART */
    // 1. Set AUXENB register to enable mini UART. Then mini UART register can be accessed.
    *AUX_ENABLES |= 1;
    // 2. Set AUX_MU_CNTL_REG to 0. Disable transmitter and receiver during configuration.
    *AUX_MU_CNTL_REG = 0;
    // 3. Set AUX_MU_IER_REG to 0. Disable interrupt because currently you don’t need interrupt.
    *AUX_MU_IER_REG = 0;
    // 4. Set AUX_MU_LCR_REG to 3. Set the data size to 8 bit.
    *AUX_MU_LCR_REG = 3;
    // 5. Set AUX_MU_MCR_REG to 0. Don’t need auto flow control.
    *AUX_MU_MCR_REG = 0;
    // 6. Set AUX_MU_BAUD to 270. Set baud rate to 115200
    *AUX_MU_BAUD_REG = 270;
    // 7. Set AUX_MU_IIR_REG to 6. No FIFO.
    *AUX_MU_IIR_REG = 6;

    /* map UART1 to GPIO pins */
    /* mini UART  -> set ALT5, PL011 UART -> set ALT0 */
    /* Configure GPFSELn register to change alternate function */
    r = *GPFSEL1;
    r &= ~((7 << 12) | (7 << 15));  // clear gpio14,15 alternate function
    r |= (2 << 12) | (2 << 15);     // alternate function 5 (TXD1 and RXD1)
    *GPFSEL1 = r;

    /* Enable pins 14 and 15 (disable pull-up/down, "floating" input pin with no pull-up or pull-down resistors) */
    *GPPUD = 0;
    /* Wait 150 cycles – this provides the required set-up time for the control signal */
    nop_delay(150);
    /* Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads you wish to modify */
    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    /* Wait 150 cycles – this provides the required set-up time for the control signal */
    nop_delay(150);
    /* flush GPIO setup */
    *GPPUDCLK0 = 0;

    // 8. Set AUX_MU_CNTL_REG to 3. Enable the transmitter and receiver.
    *AUX_MU_CNTL_REG = 3;

    // clear rx data
    while ((*AUX_MU_LSR_REG & 0x01))
        *AUX_MU_IO_REG;
}

void uart_write_char(char c) {
    while (!(*AUX_MU_LSR_REG & 0x20))
        nop_delay(1);
    *AUX_MU_IO_REG = (unsigned int)c;
}

void uart_write_string(char* str) {
    for (unsigned int i = 0; str[i] != '\0'; i++) {
        uart_write_char((char)str[i]);
    }
}

void uart_write_hex(unsigned int d) {
    unsigned int c;
    for (int i = 28; i >= 0; i -= 4) {
        /* Highest 4 bits */
        c = (d >> i) & 0xF;
        /* Translate to hex */
        c = (c > 9) ? (0x37 + c) : (0x30 + c);
        uart_write_char(c);
    }
}

void uart_read(char* buf, unsigned int size) {
    for (unsigned int i = 0; i < size; i++) {
        while (!(*AUX_MU_LSR_REG & 0x01))
            nop_delay(1);
        buf[i] = (char)(*AUX_MU_IO_REG);
    }
}

void nop_delay(unsigned int t) {
    for (unsigned int i = 0; i < t; i++)
        asm volatile("nop");
}

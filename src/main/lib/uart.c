#include "uart.h"
#include "gpio.h"
#include "mailbox.h"
#ifndef WITH_STDLIB
#include "type.h"
#else
#include <stdint.h>
#endif

void init_uart(uint32_t baud_rate) {

    register uint32_t r;

    //* Alt GPIO pin 14, 15 to UART functiona
    r = *GPFSEL1;
    r &= ((7 << 12) | (7 << 15));       // init func sel
    r |= ((2 <<12)  | (2 << 15));       // change to alt5
    *GPFSEL1 = r;

    *GPPUD = 0;                         //* enable
    r = 150;
    while(r--) { asm volatile("nop"); } //* wait for 150 cycles
    *GPPUDCLK0 = ( (1 << 14) | (1 << 15) );
    r = 150;
    while(r--) {asm volatile("nop"); } //* wait for 150 cycles
    *GPPUDCLK0 = 0;

    *AUX_ENAB |= 1;
    *AUX_MU_CNTL = 0;
    *AUX_MU_IER = 0;
    *AUX_MU_LCR = 0x3;
    *AUX_MU_MCR = 0;
    *AUX_MU_BAUD = baud_rate; //* 270 for 115200
    *AUX_MU_IIR = 0xc6;

    *AUX_MU_CNTL = 3; //* enable Tx, Rx
}


void uart_send(char c) {

    do {
        asm volatile("nop");
    } while( !((*AUX_MU_LSR) & 0x20) );
    *AUX_MU_IO = c;
}

char uart_recv() {

    do {
        asm volatile("nop");
    } while( ! ((*AUX_MU_LSR) & 0x1 ));
    char c = *AUX_MU_IO;
    return c == '\r' ? '\n' : c;
}

void uart_write(char* s) {
    while(*s) {
        if(*s == '\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

void uart_hex(uint32_t num) {

    uint32_t n;
    uart_write("0x");
    for(int c=28; c>=0; c-=4) {
        n = (num >> c)&0xf;
        n += n>9 ? 0x37 : 0x30;
        uart_send(n);
    }
    uart_write("\n");
}

char hex2a(uint8_t h) {

    if (h < 10 && h >= 0) {
        return h + '0';
    } else if( h >= 10 ) {
        return h - 10 + 'a';
    } else
        return h;

}


void uart_putc(char c) {

    do {
        asm volatile("nop");
    } while( !((*AUX_MU_LSR) & 0x20) );
    *AUX_MU_IO = c;

}


char uart_getc() {
    do {
        asm volatile("nop");
    } while( ! ((*AUX_MU_LSR) & 0x1 ));
    char c = *AUX_MU_IO;
    return c;
}


void uart_writeb(char* s, uint32_t len){
    for(int i=0;i<len;i++) {
        if(*s == '\n')
            uart_send('\r');
        uart_send(*s++);
    }
}


void pll0_uart_init() {
    register unsigned int r;

    volatile uint32_t buf[36] __attribute__((aligned(16)));
    /* initialize UART */
    *UART0_CR = 0;  // turn off UART0

    /* set up clock for consistent divisor values */
    buf[0] = 9 * 4;
    buf[1] = 0;
    buf[2] = SET_CLKRATE;  // set clock rate
    buf[3] = 12;
    buf[4] = 8;
    buf[5] = 2;        // UART clock
    buf[6] = 4000000;  // 4Mhz
    buf[7] = 0;        // clear turbo
    buf[8] = TAG_END;
    mailbox_call(8, buf);

    /* map UART0 to GPIO pins */
    r = *GPFSEL1;
    r &= ~((7 << 12) | (7 << 15));  // gpio14, gpio15
    r |= (4 << 12) | (4 << 15);     // alt0
    *GPFSEL1 = r;
    *GPPUD = 0;  // enable pins 14 and 15
    r = 150;
    while (r--) {
        asm volatile("nop");
    }
    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    r = 150;
    while (r--) {
        asm volatile("nop");
    }
    *GPPUDCLK0 = 0;  // flush GPIO setup

    *UART0_ICR = 0x7FF;  // clear interrupts
    *UART0_IBRD = 2;     // 115200 baud
    *UART0_FBRD = 0xB;
    *UART0_LCRH = 0b11 << 5;  // 8n1
    *UART0_CR = 0x301;        // enable Tx, Rx, FIFO
}

void pll0_uart_send(unsigned int c) {
    /* wait until we can send */
    do {
        asm volatile("nop");
    } while (*UART0_FR & 0x20);
    /* write the character to the buffer */
    *UART0_DR = c;
}


void pll0_uart_putc(unsigned int c) {
    /* wait until we can send */
    do {
        asm volatile("nop");
    } while (*UART0_FR & 0x20);
    /* write the character to the buffer */
    *UART0_DR = c;
}


char pll0_uart_getc() {
    char r;
    /* wait until something is in the buffer */
    do {
        asm volatile("nop");
    } while (*UART0_FR & 0x10);
    /* read it and return */
    r = (char)(*UART0_DR);
    /* convert carrige return to newline */
    return r;
}


void pll0_uart_write(char* s) {
    while(*s) {
        if(*s == '\n')
            pll0_uart_send('\r');
        pll0_uart_send(*s++);
    }
}


void pll0_uart_flush() {
    while (*UART0_FR & 0x10) {
        *UART0_DR;
    }
}


void pll0_uart_hex(uint32_t num) {

    uint32_t n;
    pll0_uart_write("0x");
    for(int c=28; c>=0; c-=4) {
        n = (num >> c)&0xf;
        n += n>9 ? 0x37 : 0x30;
        pll0_uart_send(n);
    }
    pll0_uart_write("\n");
}


void pll0_uart_writeb(char* s, uint32_t len){
    for(int i=0;i<len;i++) {
        if(*s == '\n')
            pll0_uart_send('\r');
        pll0_uart_send(*s++);
    }
}



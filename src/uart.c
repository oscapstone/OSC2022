#include "uart.h"
#include "gpio.h"
#include "type.h"

void init_uart() {

    register uint32_t r;

    *AUX_ENAB |= 1;
    *AUX_MU_CNTL = 0;
    *AUX_MU_IER = 0;
    *AUX_MU_LCR = 0x3;
    *AUX_MU_MCR = 0;
    *AUX_MU_BAUD = 0x10e;  //* 270
    *AUX_MU_IIR = 0xc6;

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

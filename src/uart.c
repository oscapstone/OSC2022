/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "uart.h"

/* Auxilary mini UART registers */
#define AUX_ENABLE      ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE+0x00215068))

/* PL011 UART registers */
#define UART0_DR        ((volatile unsigned int*)(MMIO_BASE+0x00201000))
#define UART0_FR        ((volatile unsigned int*)(MMIO_BASE+0x00201018))
#define UART0_IBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201024))
#define UART0_FBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201028))
#define UART0_LCRH      ((volatile unsigned int*)(MMIO_BASE+0x0020102C))
#define UART0_CR        ((volatile unsigned int*)(MMIO_BASE+0x00201030))
#define UART0_IMSC      ((volatile unsigned int*)(MMIO_BASE+0x00201038))
#define UART0_ICR       ((volatile unsigned int*)(MMIO_BASE+0x00201044))

#define ENABLE_IRQ_S1   ((volatile unsigned int*)(MMIO_BASE+0x0000B210))

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    register unsigned int r;
    
#ifdef USE_UART0
    *UART0_CR = 0;         // turn off UART0

    /* map UART0 to GPIO pins */
    r=*GPFSEL1;
    r&=~((7<<12)|(7<<15)); // gpio14, gpio15
    r|=(4<<12)|(4<<15);    // alt0
    *GPFSEL1 = r;
    *GPPUD = 0;            // enable pins 14 and 15
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = (1<<14)|(1<<15);
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;        // flush GPIO setup

    /* initialize UART */
    /* set up clock for consistent divisor values */
    mbox[0] = 9*4;
    mbox[1] = MBOX_REQUEST;
    mbox[2] = MBOX_TAG_SETCLKRATE; // set clock rate
    mbox[3] = 12;
    mbox[4] = 8;
    mbox[5] = 2;           // UART clock
    mbox[6] = 4000000;     // 4Mhz
    mbox[7] = 0;           // clear turbo
    mbox[8] = MBOX_TAG_LAST;
    mbox_call(MBOX_CH_PROP);

    *UART0_ICR = 0x7FF;    // clear interrupts
    *UART0_IBRD = 2;       // 115200 baud
    *UART0_FBRD = 0xB;
    *UART0_LCRH = 0x7<<4;  // 8n1, enable FIFOs
    *UART0_CR = 0x301;     // enable Tx, Rx, UART
#else
    /* map UART1 to GPIO pins */
    r=*GPFSEL1;
    r&=~((7<<12)|(7<<15)); // gpio14, gpio15
    r|=(2<<12)|(2<<15);    // alt5
    *GPFSEL1 = r;
    *GPPUD = 0;            // enable pins 14 and 15
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = (1<<14)|(1<<15);
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;        // flush GPIO setup

    // enable aux int
    *ENABLE_IRQ_S1 = (1<<29);

    /* initialize UART */
    *AUX_ENABLE |=1;       // enable UART1, AUX mini uart
    *AUX_MU_IER = 0;
    *AUX_MU_CNTL = 0;
    *AUX_MU_LCR = 3;       // 8 bits
    *AUX_MU_MCR = 0;
    *AUX_MU_IER = 0x1;     // enable receive interrupt
    *AUX_MU_IIR = 0xc6;    // clear FIFO
    *AUX_MU_BAUD = 270;    // 115200 baud
    *AUX_MU_CNTL = 3;      // enable Tx, Rx
#endif

    for (read_buf_idx = 0; read_buf_idx < READ_BUF_SIZE; read_buf_idx++)
        read_buf[read_buf_idx] = '\0';
    
    read_buf_idx = 0;
    
    write_buf_in = 0;
    write_buf_out = 0;
}

/**
 * Send a character
 */
void uart_send(unsigned int c) {

#ifdef USE_UART0
    /* wait until we can send */
    do{asm volatile("nop");}while(*UART0_FR&0x20);
    /* write the character to the buffer */
    *UART0_DR=c;
#else
    // /* wait until we can send */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
    /* write the character to the buffer */
    *AUX_MU_IO=c;
#endif
    
}

/**
 * Receive a character
 */
char uart_getc() {
#ifdef USE_UART0
    char r;
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(*UART0_FR&0x10);
    /* read it and return */
    r=(char)(*UART0_DR);
    return r;
#else
    char r;
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x01));
    /* read it and return */
    r=(char)(*AUX_MU_IO);
    return r;
#endif
    
}

/**
 * Display a string
 */
void uart_puts(char *s)
{
    while(*s) {
        /* convert newline to carrige return + newline */
        if(*s == '\n') {
            write_buf[write_buf_in] = '\r';
            write_buf_in = (write_buf_in + 1) & (WRITE_BUF_SIZE - 1);
        }
        write_buf[write_buf_in] = *s++;
        write_buf_in = (write_buf_in + 1) & (WRITE_BUF_SIZE - 1);
    }

    enable_transmit_irq();
    do{asm volatile("nop");}while(write_buf_in != write_buf_out);

    write_buf_in = 0;
    write_buf_out = 0;
}

void sync_uart_puts(char *s) {
    while(*s) {
        /* convert newline to carrige return + newline */
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

/**
 * Display a binary value in hexadecimal
 */
void uart_hex(unsigned long int d) {
    unsigned int n;
    int c;
    for(c=60;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_send(n);
    }
}

void uart_dec(long int num) {
    unsigned long divisor = 10000000000000000000;
    int digit, leading_zero = 1;

    if (num == 0) {
        uart_send('0');
        return;
    }
    else if (num < 0) {
        num = -num;
        uart_send('-');
    }

    do {
         digit = num / divisor;
         if (digit) {
             leading_zero = 0;
             num = num - digit * divisor;
         }
         divisor /= 10;
         if (!leading_zero)
            uart_send(digit + '0');
    } while (divisor);

}

void uart_sdec(char* pre, long int num, char* post)
{
    uart_puts(pre);
    uart_dec(num);
    uart_puts(post);
}

void uart_shex(char* pre, unsigned long int num, char* post)
{
    uart_puts(pre);
    uart_hex(num);
    uart_puts(post);
}

void uart_irq_handler()
{
    uint32_t value;
    value = *AUX_MU_IIR;
    value = value & 0x00000006;

    if (value == 0x4)
        receive_handler();
    else if (value == 0x2)
        transmit_handler();
    else
        exit();
}

void receive_handler()
{
    char c = *AUX_MU_IO;
    read_buf[read_buf_idx++] = c;
    if (c == '\r') {
        uart_send('\n');
    }
    uart_send(c);
}

void transmit_handler()
{
    while (write_buf_out != write_buf_in) {
        do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
        *AUX_MU_IO = write_buf[write_buf_out];

        if (write_buf_out == WRITE_BUF_SIZE - 1)
            write_buf_out = 0;
        else
            write_buf_out++;
    }

    disable_transmit_irq();
}

void enable_transmit_irq()
{
    uint32_t value = *AUX_MU_IER;
    value |= 0x2;
    *AUX_MU_IER = value;
}

void disable_transmit_irq()
{
    uint32_t value = *AUX_MU_IER;
    value &= ~(0x2);
    *AUX_MU_IER = value;
}

void disable_recieve_irq()
{
    uint32_t value = *AUX_MU_IER;
    value &= ~(0x1);
    *AUX_MU_IER = value;
}

size_t uartread(char *buf, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        buf[i] = uart_getc();
    }

    return size;
}

size_t uartwrite (const char *buf, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        uart_send(buf[i]);
    }

    return size;
}
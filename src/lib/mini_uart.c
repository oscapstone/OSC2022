#include "mini_uart.h"

char read_buf[MAX_UART_BUFFER];
char write_buf[MAX_UART_BUFFER];
int read_buf_start, read_buf_end;
int write_buf_start, write_buf_end;

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    register unsigned int r;

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

    /* initialize UART */
    *AUX_ENABLE |=1;       // enable UART1, AUX mini uart
    *AUX_MU_CNTL = 0;
    *AUX_MU_LCR = 3;       // 8 bits
    *AUX_MU_MCR = 0;
    *AUX_MU_IER = 0;
    *AUX_MU_IIR = 0xc6;    // disable interrupts
    *AUX_MU_BAUD = 270;    // 115200 baud
    *AUX_MU_CNTL = 3;      // enable Tx, Rx

    // init uart interrupt
    init_uart_interrupt();
}

/**
 * Send a character to display
 */
void uart_send(unsigned int c) {
    /* wait until we can send */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
    /* write the character to the buffer */
    *AUX_MU_IO=c;
}

/**
 * Receive a character
 */
char uart_getc() {
    char r;
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x01));
    /* read it and return */
    r=(char)(*AUX_MU_IO);
    /* convert carrige return to newline */
    return r=='\r'?'\n':r;
}

/**
 * Display a string
 */
void uart_puts(char *s) {
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
void uart_hex(unsigned int d) {
    unsigned int n;
    int c;
    for(c=28;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_send(n);
    }
}

void uart_getline(char *input) {
    char c;
    int idx;

    do {
        c = uart_getc();
        if (c <= 127) {
            uart_send(c);
            input[idx++] = c;
        }
    } while (c != '\n' && c != '\r');
    return;
}

void init_uart_interrupt() {
    *AUX_MU_IER = 0;
    read_buf_start = read_buf_end = 0;
	write_buf_start = write_buf_end = 0;
    enable_mini_uart_interrupt();
}

void enable_mini_uart_interrupt() {
    *ENABLE_IRQs1 = AUX_IRQ;
}

void disable_mini_uart_interrupt() {
    *DISABLE_IRQs1 = AUX_IRQ;
}

void enable_write_interrupt() { 
	*AUX_MU_IER |= 0x2; 
}

void disable_write_interrupt() { 
	*AUX_MU_IER &= ~(0x2); 
}

void enable_read_interrupt() {
    *AUX_MU_IER = 1;
}

void disable_read_interrupt() {
    *AUX_MU_IER = 0;
}

void mini_uart_interrupt_handler() {
    disable_mini_uart_interrupt();

    if (*AUX_MU_IIR & 0x4) // read
    {
        while (*AUX_MU_LSR & 0x1) {
            char c = (char)(*AUX_MU_IO);
            read_buf[read_buf_end++] = c;

            if (read_buf_end == MAX_UART_BUFFER) read_buf_end = 0;
        }
    }
    else if (*AUX_MU_IIR & 0x2) // write
    {

        while (1)
        {
            if (write_buf_start == write_buf_end) {
                disable_write_interrupt();
                break;
            }
            uart_send(write_buf[write_buf_start++]);
        }


    }
    enable_mini_uart_interrupt();
}

char uart_async_getc() {
    enable_read_interrupt();
    // until there is new data, blocking
    while (read_buf_start == read_buf_end) {
        asm volatile ("nop");
    }
    char c = read_buf[read_buf_start++];
    if (read_buf_start == MAX_UART_BUFFER) read_buf_start = 0;
    disable_read_interrupt();
    return c;
}

void uart_async_puts(char *s) {
    for (int i = 0; s[i]; i++) {
        if (s[i] == '\n') {
            write_buf[write_buf_end++] = '\r';
            break;
        }
        write_buf[write_buf_end++] = s[i];
        if (write_buf_end == MAX_UART_BUFFER) {
            write_buf_end = 0;
        }
    }
    enable_write_interrupt();
}
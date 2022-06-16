#include "uart.h"
#include "address.h"
#include "utils.h"
#include "type.h"
#include "interrupt.h"


char tx_buffer[MAX_BUF_SIZE] = {};
unsigned long long tx_buffer_widx = 0; //write index
unsigned long long tx_buffer_ridx = 0; //read index
char rx_buffer[MAX_BUF_SIZE] = {};
unsigned long long rx_buffer_widx = 0;
unsigned long long rx_buffer_ridx = 0;

void uart_init() {
    register unsigned int r;

    /* initialize mini UART */
    *AUX_ENABLE    |= 0x01;    // enable UART1
    *AUX_MU_CNTL    = 0x00;    // disable Tx, Rx during configuration
    *AUX_MU_LCR     = 0x03;    // data size is 8 bits
    *AUX_MU_MCR     = 0x00;    // no auto flow control
    *AUX_MU_IER     = 0x00;    // disable interrupts
    *AUX_MU_IIR     = 0xc6;    // no FIFO
    *AUX_MU_BAUD    = 270;     // 115200 baud = clock / (8 x (MU_BAUD + 1))

    /* map UART1 to GPIO pins */
    r = *GPFSEL1;
    r &= ~((7 << 12) | (7 << 15)); // gpio14, gpio15
    r |= (2 << 12) | (2 << 15);    // alt5
    *GPFSEL1 = r;
    *GPPUD = 0;              // enable pins 14 and 15
    delay_cycle(150);
    
    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    delay_cycle(150);
    
    *GPPUDCLK0 = 0x00;       // flush GPIO setup
    *AUX_MU_CNTL = 0x03;     // enable Tx, Rx
}

void uart_enable() {
    *AUX_MU_CNTL = 0x03; 
}

void uart_disable() {
    *AUX_MU_CNTL = 0x00; 
}

void uart_putc(unsigned int c) {
    /* wait for ready */
    do {
        asm volatile("nop");
    } while(!(*AUX_MU_LSR & 0x20));
    
    *AUX_MU_IO = c; // write character
}

void uart_puts(char *s) {
    while(*s) {
        /* convert newline to carrige return + newline */
        if(*s == '\n') {
            uart_putc('\r');
        }

        uart_putc(*s++);
    }
}

void uart_newline() {
    uart_puts("\n");
}

void uart_dem() {
    uart_puts("\n=================================\n");
}

void uart_prefix() {
    uart_puts("\n>> ");
}

void uart_hex(uint64 d) {
    unsigned int n;
    int c;
    uart_puts("0x");
    for(c = 28; c >= 0; c -= 4) {
        // get highest tetrad
        n = (d >> c) & 0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n += n > 9 ? 0x37 : 0x30;
        uart_putc(n);
    }
}

void uart_num(int64 d) {
    unsigned int n;
    char s[16];
    int i;

    if(d < 0) {
        d *= -1;
        uart_putc('-');
    }

    for(i = 0; d > 0; i++) {
        n = d % 10 + 0x30;
        s[i] = n;
        d /= 10;
    }

    if(i == 0) {
        uart_putc('0');
    }
    else {
        while(i--) {
            uart_putc(s[i]);
        }
    }
}

char uart_getb() {
    char r;
    /* wait for input */
    do {
        asm volatile("nop");
    } while(!(*AUX_MU_LSR & 0x01));

    r = (char)(*AUX_MU_IO); // read input character

    return r;
}

char uart_getc() {
    char r;
    /* wait for input */
    do {
        asm volatile("nop");
    } while(!(*AUX_MU_LSR & 0x01));

    r = (char)(*AUX_MU_IO); // read input character
    
    /* convert carrige return to newline, may not be necessary according to your serial console */
    return (r == '\r') ? '\n' : r;
}

unsigned int uart_getn() {
    int num = 0;
    char c;
    do {
        c = uart_getc();
        
        if(c != '\n') {
            uart_putc(c);
            num = num * 10 + c - '0';
        }
        else {
            uart_newline();
        }
    } while(c != '\n');

    return num;
}

char* uart_img_receiver(char* address) {
    // receive image size (order: little, type: integer, unit: bytes)
    uart_init();
    uart_getc(); // flush init signal(0xE0)
    
    unsigned int size = 0;
    size = uart_getb() |
           uart_getb() <<  8 | 
           uart_getb() << 16 |
           uart_getb() << 24;
    
    // echo size info
    uart_hex((uint64)address);
    uart_putc(' ');
    uart_puts("Image size = ");
    uart_num(size); 
    uart_puts("B\n");

    // write image binary to pre-defined address
    char* kernel = address;
    while(size--) {
        *(kernel++) = uart_getb();
        // for stable communication
        uart_num(size);
        uart_newline();
    }

    // echo finish message
    uart_puts("Receive image done, jump to address ");
    uart_hex((uint64)address);
    uart_dem();
    delay_ms(2000); // wait message to be sent before restart new kernel
        
    return address;
}

// ############## Asynchronize ############## 

void uart_interrupt_r_handler()
{
    //read buffer full
    if ((rx_buffer_widx + 1) % MAX_BUF_SIZE == rx_buffer_ridx)
    {
        disable_uart_r_interrupt(); //disable read interrupt when read buffer full
        return;
    }
    rx_buffer[rx_buffer_widx++] = uart_getc();
    if (rx_buffer_widx >= MAX_BUF_SIZE)
        rx_buffer_widx = 0;

    enable_uart_r_interrupt();
}

void uart_interrupt_w_handler() //can write
{
    // buffer empty
    if (tx_buffer_ridx == tx_buffer_widx)
    {
        disable_uart_w_interrupt(); // disable w_interrupt to prevent interruption without any async output
        return;
    }
    uart_putc(tx_buffer[tx_buffer_ridx++]);
    if (tx_buffer_ridx >= MAX_BUF_SIZE)
        tx_buffer_ridx = 0; // cycle pointer

    enable_uart_w_interrupt();
}

void uart_async_puts(char *s) {
    while(*s) {
        /* convert newline to carrige return + newline */
        if(*s == '\n') {
            uart_async_putc('\r');
        }

        uart_async_putc(*s++);
    }
}

unsigned int uart_async_putc(char c) {
    // full buffer wait
    while((tx_buffer_widx + 1) % MAX_BUF_SIZE == tx_buffer_ridx) {
        // start asynchronous transfer
        enable_uart_w_interrupt();
        // uart_puts("\n TX fails, uart async tx buffer is full\n");
        // return 1;
    }

    
    // critical section
    lock_interrupt();
    tx_buffer[tx_buffer_widx++] = c;
    if (tx_buffer_widx >= MAX_BUF_SIZE)
        tx_buffer_widx = 0; // cycle pointer

    // start asynchronous transfer
    unlock_interrupt();
    
    // enable interrupt to transfer
    enable_uart_w_interrupt();
    return 0;
}

void uart_async_newline() {
    uart_async_puts("\n");
}

void uart_async_dem() {
    uart_async_puts("\n=================================\n");
}

void uart_async_prefix() {
    uart_async_puts("\n>> ");
}

void uart_async_hex(uint64 d) {
    unsigned int n;
    int c;
    uart_async_puts("0x");
    for(c = 28; c >= 0; c -= 4) {
        // get highest tetrad
        n = (d >> c) & 0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n += n > 9 ? 0x37 : 0x30;
        uart_async_putc(n);
    }
}

void uart_async_num(int64 d) {
    unsigned int n;
    unsigned int s[16];
    int i;
    for(i = 0; d > 0; i++) {
        n = d % 10 + 0x30;
        s[i] = n;
        d /= 10;
    }

    if(i == 0) {
        uart_async_putc('0');
    }
    else {
        while(i--) {
            uart_async_putc(s[i]);
        }
    }
}

char uart_async_getc()
{
    enable_uart_r_interrupt();
    // while buffer empty
    if (rx_buffer_ridx == rx_buffer_widx) 
    {
        return NULL;
    }

    // critical section
    lock_interrupt();
    char r = rx_buffer[rx_buffer_ridx++];

    if (rx_buffer_ridx >= MAX_BUF_SIZE)
        rx_buffer_ridx = 0;

    unlock_interrupt();

    return r;
}

// enable uart interrupt

void enable_uart_interrupt()
{
    enable_uart_r_interrupt();
    enable_uart_w_interrupt();
    *IRQS1 |= 1 << 29;
}

void enable_uart_r_interrupt()
{
    *AUX_MU_IER |= 1; // read interrupt
}

void enable_uart_w_interrupt() {
    *AUX_MU_IER |= 2; // write interrupt
}

// disable uart interrupt

void disable_uart_interrupt()
{
    disable_uart_r_interrupt();
    disable_uart_w_interrupt();
}

void disable_uart_r_interrupt()
{
    *AUX_MU_IER &= ~(1);
}

void disable_uart_w_interrupt()
{
    *AUX_MU_IER &= ~(2);
}

//

void raiseError(char *message) {
    uart_puts("[Error] ");
    uart_puts(message);
    while(1);
}

int uart_printf(char *fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    char buf[MAX_BUF_SIZE];
    // we don't have memory allocation yet, so we
    // simply place our string after our code
    char *s = (char *)buf;
    // use sprintf to format our string
    int count = vsprintf(s, fmt, args);
    // print out as usual

    while (*s)
    {
        if(*s == '\n') {
            uart_putc('\r');
        }
        uart_putc(*s++);
    }
    __builtin_va_end(args);
    return count;
}

int uart_async_printf(char *fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    char buf[MAX_BUF_SIZE];
    // we don't have memory allocation yet, so we
    // simply place our string after our code
    char *s = (char *)buf;
    // use sprintf to format our string
    int count = vsprintf(s, fmt, args);
    // print out as usual
    while (*s)
    {
        if(*s == '\n') {
            uart_async_putc('\r');
        }
        uart_async_putc(*s++);
    }
    __builtin_va_end(args);
    return count;
}
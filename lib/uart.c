#include "address.h"
#include "utils.h"

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

// Send a character
void uart_putc(unsigned int c) {
    /* wait for ready */
    do {
        asm volatile("nop");
    } while(!(*AUX_MU_LSR & 0x20));
    
    *AUX_MU_IO = c; // write character
}

// Send a string
void uart_puts(char *s) {
    while(*s) {
        /* convert newline to carrige return + newline */
        if(*s == '\n') {
            uart_putc('\r');
        }

        uart_putc(*s++);
    }
}

// Show new line
void uart_newline() {
    uart_puts("\n");
}

// Show split line
void uart_dem() {
    uart_puts("\n=================================\n");
}

// show prefix >>
void uart_prefix() {
    uart_puts("\n>> ");
}

// Show hex value
void uart_hex(unsigned int d) {
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

// Show dec value
void uart_num(unsigned int d) {
    unsigned int n;
    unsigned int s[16];
    int i;
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

// Receive a binary
char uart_getb() {
    char r;
    /* wait for input */
    do {
        asm volatile("nop");
    } while(!(*AUX_MU_LSR & 0x01));

    r = (char)(*AUX_MU_IO); // read input character

    return r;
}

// Receive a character
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

// Receive a number
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

// Get a new kernel via uart
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
    uart_hex(address);
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
    uart_hex(address);
    uart_dem();
    delay_ms(2000); // wait message to be sent before restart new kernel
        
    return address;
}
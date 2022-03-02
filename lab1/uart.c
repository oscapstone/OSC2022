#include "uart.h"
#include "gpio.h"

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

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    register unsigned int r;

    /* initialize UART */
    *AUX_ENABLE |=1;       // enable UART1, AUX mini uart
    *AUX_MU_CNTL = 0;      // Disable Tx and Rx during configuration
    *AUX_MU_IER = 0;       // Disable interrupt
    *AUX_MU_LCR = 3;       // Set the data size to 8 bit
    *AUX_MU_MCR = 0;       // Don’t need auto flow control
    *AUX_MU_BAUD = 270;    // Set baud rate to 115200, 250M/(8*271) = 115,313.6531365314
    *AUX_MU_IIR = 0xc6;    // clear FIFO's transmit and receive 0b110

    /* map UART1 to GPIO pins */
    r = *GPFSEL1;
    r &= ~((7<<12)|(7<<15)); // zero fill gpio14, gpio15, bit position
    r |= (2<<12)|(2<<15);    // set alt5 on gpio14, gpio15
    *GPFSEL1 = r;
    *GPPUD = 0;              // disable pull-up/down
    r=150; while(r--) { asm volatile("nop"); }          // wait 150 cycles for gpio setup control signal
    *GPPUDCLK0 = (1<<14)|(1<<15);                       // assert clock control signal on pin 14, 15
    r=150; while(r--) { asm volatile("nop"); }          // wait 150 cycles
    *GPPUDCLK0 = 0;        // flush GPIO setup          // remove clock
    
    *AUX_MU_CNTL = 3;      // enable Tx, Rx 
}

/**
 * Send a character
 */
void uart_send(unsigned int c) {

    // if(c == '\n') {
    //     do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
    //     *AUX_MU_IO=' ';
    //     do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
    //     *AUX_MU_IO='n';
    // }
    // else if (c == '\r') {
    //     do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
    //     *AUX_MU_IO=' ';
    //     do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
    //     *AUX_MU_IO='r';
    // }
    

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

    // uart_hex(r);
    // uart_send(' ');
    
    /* echo */
    if (r == '\r'){
        uart_send('\r');
        uart_send('\n');
    }
    else if (r == '\x7f') {
        uart_send('\b');
        uart_send(' ');
        uart_send('\b');
    }
    else {
        uart_send(r);
    }
    
    /* convert carrige return to newline */
    return r=='\r'?'\n':r;
}

/**
 * Recieve a line
 */
char* uart_gets(char *buf) {
    int count;
    char c;
    char *s;
    for (s = buf,count = 0; (c = uart_getc()) != '\n' && count!=BUFFER_SIZE-1 ;count++) {
        *s = c;
        if(*s=='\x7f') {
            count--;
            if(count==-1) {
                uart_send(' ');
                continue;
            }
            s--;
            count--;
            continue;
        }
        s++;
    }
	*s = '\0';
	return buf;
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

/**
 * display int
 */
int uart_int(int d) {
    char buf[BUFFER_SIZE];
    char tmp;
    int digits = 0, n = d;
    
    if (d == 0){
        uart_send('0');
        return 0;
    }

    if (n < 0) {
        n = -n;
    }

    while (n != 0) {
        digits++;
        n /= 10;
    }

    n = d;
    if (n < 0) {
        n = -n;
    }

    if ( digits > BUFFER_SIZE - 1) {
        uart_puts("Exceed buffer size.\n");
        return -1;
    }
    
    buf[digits] = '\0';
    for (int i = digits-1; i >= 0; i--) {
        tmp = n%10 + '0';
        n /= 10;
        buf[i] = tmp;
    }
    if (d<0) {
        uart_send('-');
    }
    uart_puts(buf);
    return 0;
}

/**
 * 
 */
void uart_printf(){

}


/**
 * 
 * 0000001B 000005B [00000041 A         ^
 * 0000001B 000005B [00000042 B         
 * 0000001B 000005B [00000044 D         <
 * 0000001B 000005B [00000043 C         >
 * 
 */
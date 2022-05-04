#include "uart.h"
#include "gpio.h"
#include "utils.h"
#include "command.h"

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

#define ENB_IRQS1 		((volatile unsigned int*)(MMIO_BASE+0x0000b210))
#define DISABLE_IRQS1 	((volatile unsigned int*)(MMIO_BASE+0x0000b21c))
#define AUX_IRQ (1 << 29)

// #define ASYNC_UART
#define ASYNC_BUFFER_SIZE 128
char async_read_buffer[ASYNC_BUFFER_SIZE];
char async_write_buffer[ASYNC_BUFFER_SIZE];
unsigned int async_read_start, async_read_end;
unsigned int async_write_start, async_write_end;

void enable_uart_interrupt() { *ENB_IRQS1 = AUX_IRQ; }

void disable_uart_interrupt() { *DISABLE_IRQS1 = AUX_IRQ; }

void enable_transmit_interrupt() { *AUX_MU_IER |= 0x2; }

void disable_transmit_interrupt() { *AUX_MU_IER &= ~(0x2); }

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    register unsigned int r;

    /* initialize UART */
    *AUX_ENABLE |=1;       // Set AUXENB register to enable mini UART. Then mini UART register can be accessed
    *AUX_MU_CNTL = 0;      // Set AUX_MU_CNTL_REG to 0. Disable transmitter and receiver during configuration
    #ifdef ASYNC_UART
    *AUX_MU_IER = 1;       // Set AUX_MU_IER_REG to 1. enable interrupt
    #else
    *AUX_MU_IER = 0;       // Set AUX_MU_IER_REG to 0. Disable interrupt because currently you don’t need interrupt
    #endif
    *AUX_MU_LCR = 3;       // Set AUX_MU_LCR_REG to 3. Set the data size to 8 bit
    *AUX_MU_MCR = 0;       // Set AUX_MU_MCR_REG to 0. Don’t need auto flow control
    *AUX_MU_BAUD = 270;    // Set AUX_MU_BAUD to 270. Set baud rate to 115200
    *AUX_MU_IIR = 6;       // Set AUX_MU_IIR_REG to 6. No FIFO
    
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
    *AUX_MU_CNTL = 3;      // Set AUX_MU_CNTL_REG to 3. Enable the transmitter(Tx) and receiver(Rx)
    #ifdef ASYNC_UART
    enable_uart_interrupt();
    #endif
}

/**
 * Send a character
 */
void uart_send(unsigned int c) {
    /* wait until we can send */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
    /* write the character to the buffer */
    *AUX_MU_IO=c;
    if (c == '\n') {
        /* only on raspberry pi */
        do {asm volatile("nop");} while( ! ( *AUX_MU_LSR&0x20 ));

        *AUX_MU_IO = '\r';
    }
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

int uart_gets(char* s,int size,int display){
	for(int i=0;;++i){
		if(i==size){
			uart_puts("buffer overflow!\n");
			return i;
		}

		s[i]=uart_getc();
		if(display)uart_send(s[i]);

		if(s[i]=='\n'){
			s[i]=0;
			return i;
		}
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
 * Display a string
 */
void printf(char *fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    // we don't have memory allocation yet, so we
    // simply place our string after our code
    // char *s = (char*)&_end;
    char stmp[100];
    char *s = stmp;
    // use sprintf to format our string
    vsprintf(s,fmt,args);
    // print out as usual
    while(*s) {
        /* convert newline to carrige return + newline */
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

void handle_uart_irq() {
    disable_uart_interrupt();
	if((*AUX_MU_IIR&0x06) == 0x04) {
        // uart read interrupt handler
		while(*AUX_MU_LSR&0x01) {
            char r = *AUX_MU_IO;
			async_read_buffer[async_read_end++] = r;
			if(async_read_end == ASYNC_BUFFER_SIZE)
				async_read_end = 0;
		}
	}
	else if((*AUX_MU_IIR&0x06) == 0x02) {
        // uart write interrupt handler
		while(*AUX_MU_LSR&0x20) {
			if(async_write_start == async_write_end) {
                disable_transmit_interrupt();
			    break;
            }
			char r = async_write_buffer[async_write_start++];
			*AUX_MU_IO = r;
			if(async_write_start == ASYNC_BUFFER_SIZE)
				async_write_start = 0;
		}
	}
    enable_uart_interrupt();
}

char async_uart_getc() {
    while (async_read_start == async_read_end) {
        delay(1);
    }
    char r = async_read_buffer[async_read_start++];
    if (async_read_start == ASYNC_BUFFER_SIZE)
        async_read_start = 0;
    return r=='\r'?'\n':r;
}

void async_uart_send(const char *str, unsigned long size) {
    for (int i = 0; i < size; i++) {
        async_write_buffer[async_write_end++] = str[i];
        if (async_write_end == ASYNC_BUFFER_SIZE) async_write_end = 0;
    }
    enable_transmit_interrupt();
}

void async_uart_puts(char *str) {
    for (int i = 0; str[i]; i++) {
        async_write_buffer[async_write_end++] = str[i];
        if (str[i] == '\n')
            async_write_buffer[async_write_end++] = '\r';
        if (async_write_end == ASYNC_BUFFER_SIZE) async_write_end = 0;
    }
}

void test_async_write(){
	async_uart_puts("test\n");
    enable_transmit_interrupt();
}
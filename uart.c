#include "gpio.h"
#include "utils.h"
#include "command.h"
#include "shell.h"

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

#define ASYNC_BUFFER_SIZE 128
static char async_uart_buffer[ASYNC_BUFFER_SIZE];
static unsigned int wr_buffer_index = 0;
static unsigned int rd_buffer_index = 0;

void enable_uart_interrupt() { *ENB_IRQS1 = AUX_IRQ; }

void disable_uart_interrupt() { *DISABLE_IRQS1 = AUX_IRQ; }

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    register unsigned int r;

    /* initialize UART */
    *AUX_ENABLE |=1;       // Set AUXENB register to enable mini UART. Then mini UART register can be accessed
    *AUX_MU_CNTL = 0;      // Set AUX_MU_CNTL_REG to 0. Disable transmitter and receiver during configuration
    
    *AUX_MU_IER = 1;       // Set AUX_MU_IER_REG to 0. Disable interrupt because currently you don’t need interrupt

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
    
    enable_uart_interrupt();
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
    // printf("irq_interrupt\n");
    disable_uart_interrupt();
	if((*AUX_MU_IIR&0x06) == 0x04) {
		while(*AUX_MU_LSR&0x01) {
            char async_input_char = *AUX_MU_IO;
            // async_input_char = *AUX_MU_IO;
			async_uart_buffer[wr_buffer_index] = async_input_char;
            wr_buffer_index = parse(async_input_char, wr_buffer_index);

            if (async_input_char == '\r') {
                // Enter
                async_uart_buffer[wr_buffer_index] = 0;
                wr_buffer_index = 0;

                parse_command(async_uart_buffer);
                uart_puts("# ");
            }
            // wr_buffer_index++;
			// if(wr_buffer_index == ASYNC_BUFFER_SIZE)
			// 	wr_buffer_index = 0;
		}
	}
	else if((*AUX_MU_IIR&0x06) == 0x02) {
		while(*AUX_MU_LSR&0x20) {
			if(rd_buffer_index == wr_buffer_index)
			    break;
			char c = async_uart_buffer[rd_buffer_index++];
			*AUX_MU_IO = c;
			if(rd_buffer_index == ASYNC_BUFFER_SIZE)
				rd_buffer_index = 0;
		}
	}
    enable_uart_interrupt();
}

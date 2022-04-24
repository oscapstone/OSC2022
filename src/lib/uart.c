#include "uart.h"

/* Asynchronous Read and Write */
char read_buffer[READ_BUF_SIZE];
char write_buffer[WRITE_BUF_SIZE];

unsigned int read_head;
unsigned int write_head;
unsigned int read_tail;
unsigned int write_tail;

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
    *AUX_ENABLE |=1;       // enable UART1, AUX  uart
    *AUX_MU_CNTL = 0;
    *AUX_MU_LCR = 3;       // 8 bits
    *AUX_MU_MCR = 0;
    *AUX_MU_IER = 0;
    *AUX_MU_IIR = 0xc6;    // disable interrupts
    *AUX_MU_BAUD = 270;    // 115200 baud
    *AUX_MU_CNTL = 3;      // enable Tx, Rx

    asyn_uart_init();
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

void uart_put_int(unsigned long num){
    if(num == 0) uart_send('0');
    else{
        if(num > 10) uart_put_int(num / 10);
        uart_send(num % 10 + '0');
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

void uart_getline(char *input) {
    char c;
    int idx = 0;

    do {
        c = uart_getc();
        if (c <= 127) {
            uart_send(c);
            input[idx++] = c;
        }
    } while (c != '\n' && c != '\r');
    return;
}

// void uart_put (char c) {

    // /* Wait for transmitter ready to receive data */
//     while ((*AUX_MU_LSR & 0x20) == 0) asm volatile ("nop");
    
//     /* Put data */
//     *AUX_MU_IO = c;

//     return;
// }
//////////////////////////////////////////////////////////////////////////////////////////////

void asyn_uart_init () {
    disable_read_interrupt();

    // enable_read_interrupt();
    
    read_head  = 0;
    write_head = 0;
    read_tail  = 0;
    write_tail = 0;

    enable_uart_interrupt();
    /* Clear buffer */
    // for (int i = 0; i < READ_BUF_SIZE; i++) read_buffer[i] = 0;
    // for (int i = 0; i < WRITE_BUF_SIZE; i++) write_buffer[i] = 0;

    // return;
}

void enable_uart_interrupt() {
    *IRQs_1_ENABLE = (1 << 29);
}

void disable_uart_interrupt() {
    *IRQs_1_DISABLE = (1 << 29);
}

void enable_read_interrupt() {
    *AUX_MU_IER = 1;
}

void disable_read_interrupt() {
    *AUX_MU_IER = 0;
}

void enable_write_interrupt() { 
	*AUX_MU_IER |= 0x2; 
}

void disable_write_interrupt() { 
	*AUX_MU_IER &= ~(0x2); 
}

// void set_uart_rx_int (bool enable) {

//     unsigned int d;
//     d = *AUX_MU_IER;

//     if (enable)
//     {
//         d = d | 0x1;
//     }
//     else
//     {
//         d = d & ~(0x1);
//     }

//     *AUX_MU_IER = d;

//     return;
// }

// void set_uart_tx_int (bool enable) {

//     unsigned int d;
//     d = *AUX_MU_IER;

//     if (enable)
//     {
//         d = d | 0x2;
//     }
//     else
//     {
//         d = d & ~(0x2);
//     }

//     *AUX_MU_IER = d;

//     return;
// }

char asyn_uart_get () {

    enable_read_interrupt();

    char c;

    while (read_head == read_tail)  
    {
        asm volatile ("nop");
    }/* Wait for data */ ;

    c = read_buffer[read_head++];

    if (read_head == READ_BUF_SIZE)
        read_head = 0;

    disable_read_interrupt();

    return c;
}

void asyn_uart_put (char c) {
    write_buffer[write_head] = c;

    write_head = (write_head + 1) & (WRITE_BUF_SIZE - 1);

    /* Enable TX interrupt */
    enable_write_interrupt();

}

void asyn_uart_puts (char *s) {
    for (int i = 0; s[i]; i++) 
	{
		if (s[i] == '\n') 
			write_buffer[write_tail++] = '\r';

		write_buffer[write_tail++] = s[i];		
		if (write_tail == WRITE_BUF_SIZE) 
			write_tail = 0;
	}

    /* Enable TX interrupt */
    enable_write_interrupt();

}

void uart_irq_handler () {

    unsigned int is_rx_irq;
    unsigned int is_tx_irq;

    disable_uart_interrupt();


    is_rx_irq = (*AUX_MU_IIR & 0x4); // Receiver holds valid byte
    is_tx_irq = (*AUX_MU_IIR & 0x2); // Transmit holding register empty

    if (is_rx_irq) //read
    {
       while (*AUX_MU_LSR & 0x1)
		{
			char c = (char)(*AUX_MU_IO);
			read_buffer[read_tail++] = c;

			if (read_tail == READ_BUF_SIZE) 
				read_tail = 0;
		}
    }
    else if (is_tx_irq) //write
    {
        while (1)
        {
            if (write_head == write_tail) {
                disable_write_interrupt();
                break;
            }
            // uart_puts("3333");
            uart_send(write_buffer[write_head++]);
        }
    }
    
    enable_uart_interrupt();

    return;
}

char async_uart_getc() 
{
    enable_read_interrupt();
	// wait until there are new data
	while (read_head == read_tail) 
	{
		asm volatile("nop");
	}

	char c = read_buffer[read_head++];
	if (read_head == READ_BUF_SIZE) 
		read_head = 0;

    // uart_puts("+++++++++++");

    disable_read_interrupt();
	
	return c;
}
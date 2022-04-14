
#include "gpio.h"
#include "uart.h"
#include "string.h"

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    register unsigned int reg;

    /* initialize UART */
    *AUXENB     |= 1;       /* enable mini UART */
    *AUX_MU_CNTL_REG     = 0;       /* Disable transmitter and receiver during configuration. */

    *AUX_MU_IER_REG      = 0;       /* Disable interrupt */
    *AUX_MU_LCR_REG      = 3;       /* Set the data size to 8 bit. */
    *AUX_MU_MCR_REG      = 0;       /* Don’t need auto flow control. */
    *AUX_MU_BAUD     = 270;     /* 115200 baud */
    *AUX_MU_IIR_REG      = 6;       /* No FIFO */
    // *AUX_MU_IIR      = 0xc6;       /* No FIFO */

    /* map UART1 to GPIO pins */
    reg = *GPFSEL1;
    reg &= ~((7<<12)|(7<<15));  /* address of gpio 14, 15 */
    reg |=   (2<<12)|(2<<15);   /* set to alt5 */

    *GPFSEL1 = reg;

    *GPPUD = 0;                 /* enable gpio 14 and 15 */
    reg=150;
    while ( reg-- )
    { 
        asm volatile("nop"); 
    }
    
    *GPPUDCLK0 = (1<<14)|(1<<15);
    reg=150; 
    while ( reg-- )
    {
        asm volatile("nop");
    }
    
    *GPPUDCLK0 = 0;             /* flush GPIO setup */

    *AUX_MU_CNTL_REG = 3;           // Enable the transmitter and receiver.

	*AUXENB |= 1;		   // enable mini UART
	*AUX_MU_CNTL_REG = 0;  // disable transmitter, receiver during configuration

    *AUX_MU_IER_REG = 1;   // enable receive interrupts
	
	*AUX_MU_LCR_REG = 3;   // enable 8 bit mode
	*AUX_MU_MCR_REG = 0;   // set RTS line to be always high
	*AUX_MU_BAUD = 270;    // set baud rate to 115200
	// comment this line to avoid weird character
	// *AUX_MU_IIR_REG = 0xc6;	// no FIFO
	*AUX_MU_CNTL_REG = 3;  // enable transmitter and receiver back


	read_buf_start = read_buf_end = 0;
	write_buf_start = write_buf_end = 0;
	enable_uart_interrupt();
}

void enable_uart_interrupt(){
    *ENB_IRQS1 = (1 << 29); //Enable IRQs 1
}

void disable_uart_interrupt(){
    *DISABLE_IRQS1 = (1 << 29); //Disable IRQs 1
}
//If this bit is clear no transmit interrupts are generated
//If this bit is clear no receive interrupts are generated.
void assert_transmit_interrupt() { *AUX_MU_IER_REG |= 0x2; }

void clear_transmit_interrupt() { *AUX_MU_IER_REG &= ~(0x2); }

void uart_handler() {
  disable_uart_interrupt();
  int rx = (*AUX_MU_IIR_REG & 0x4); //Receiver holds valid byte 
  int tx = (*AUX_MU_IIR_REG & 0x2); //Transmit holding register empty 
  if (rx) {
    char c = (char)(*AUX_MU_IO); //primary used to write data to and read data from the UART FIFOs
    read_buf[read_buf_end++] = c;
    if (read_buf_end == MAX_BUFFER_LEN) read_buf_end = 0;
  } 
  else if (tx) {
    while (*AUX_MU_LSR & 0x20) { //This bit is set if the transmit FIFO can accept at least one byte.
      if (write_buf_start == write_buf_end) {
        clear_transmit_interrupt();
        break;
      }
      char c = write_buf[write_buf_start++];
      *AUX_MU_IO = c;
      if (write_buf_start == MAX_BUFFER_LEN) write_buf_start = 0;
    }
  }
  enable_uart_interrupt();
}

char uart_async_getc() {
  // wait until there are new data
  while (read_buf_start == read_buf_end) {
    asm volatile("nop");
  }
  char c = read_buf[read_buf_start++];
  if (read_buf_start == MAX_BUFFER_LEN) read_buf_start = 0;
  // '\r' => '\n'
  return c == '\r' ? '\n' : c;
}

void uart_async_puts(char *str) {
  for (int i = 0; str[i]; i++) {
    if (str[i] == '\r') write_buf[write_buf_end++] = '\n';
    write_buf[write_buf_end++] = str[i];
    if (write_buf_end == MAX_BUFFER_LEN) write_buf_end = 0;
  }
  //uart_puts(write_buf);
  assert_transmit_interrupt();
}

/**
 * Send a character
 */
void uart_send(unsigned int c)
{
    /* Wait until we can send */
    do {
        
        asm volatile("nop");

    } while( ! ( *AUX_MU_LSR&0x20 ));
    
    /* write the character to the buffer */   
    *AUX_MU_IO = c;

    if ( c == '\n' ) 
    {
        do {
            
            asm volatile("nop");

        } while( ! ( *AUX_MU_LSR&0x20 ));
        
        *AUX_MU_IO = '\r';
    }
}

/**
 * Receive a character
 */
char uart_getc() {

    char r;
    
    /* wait until something is in the buffer */
    do{
        
        asm volatile("nop");
        
    } while ( ! ( *AUX_MU_LSR&0x01 ) );

    /* read it and return */
    r = ( char )( *AUX_MU_IO );

    /* convert carrige return to newline */
    return r == '\r' ? '\n' : r;
}

/**
 * Display a string
 */
void uart_puts(char *s)
{
    while( *s )
    {
        /* convert newline to carrige return + newline */
    
        //if(*s=='\n')
        //    uart_send('\r');

        uart_send(*s++);

    }
}

int uart_get_int(){
    char c;
    int num = 0;
    while((c = uart_getc())!='\n'){
        uart_send(c);
        num = num*10 + c - '0';
    }
    return num;
}

void uart_put_hex(unsigned long num){
    char str[10];
    unsign_itoa(num, str);
    uart_puts(str);
}

void uart_put_int(unsigned long num){
    if(num == 0) uart_send('0');
    else{
        if(num > 10) uart_put_int(num / 10);
        uart_send(num % 10 + '0');
    }
}

void test_uart_async(){
	/*  need time */
    int reg=1500;
    while ( reg-- )
    { 
        asm volatile("nop"); 
    }

	uart_async_puts("test\r\n");
}

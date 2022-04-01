#include "uart.h"
#include "printf.h"
#include "timer.h"

void uart_init()
{
  register unsigned int r;

  r=*GPFSEL1;
  r&=~((7<<12)|(7<<15)); // gpio14, gpio15
  r|=(2<<12)|(2<<15);    // alt5  010010
  *GPFSEL1 = r;
  *GPPUD = 0;            // enable pins 14 and 15
  r=150; 
  while(r--) {
    asm volatile("nop");
  }
  *GPPUDCLK0 = (1<<14)|(1<<15);
  r=150; 
  while(r--) { 
    asm volatile("nop"); 
  }
  *GPPUDCLK0 = 0;        // flush GPIO setup

  *AUX_ENABLE |=1;       // enable UART1, AUX mini uart
  *AUX_MU_CNTL = 0;
  *AUX_MU_LCR = 3;       // set 8 bits
  *AUX_MU_MCR = 0;
  *AUX_MU_IER = 3;       // enalbe interrupt  // no use read interrupt so use 2, if want to enable read interrupt plz use 3
  *IRQS1 |= 1 << 29;
  *AUX_MU_IIR = 0x06;    // disable interrupts
  *AUX_MU_BAUD = 270;    // 115200 baud, system clock freq = 250MHz

  *AUX_MU_CNTL = 3;      // enable Tx, Rx
}

void uart_send(unsigned int c) {
  do{
    asm volatile("nop");
  }while(!(*AUX_MU_LSR&0x20)); // wait until we can send
  *AUX_MU_IO=c; // write the character to the buffer
}

char uart_getc() {
  char r;
  
  do{
    asm volatile("nop");
  }while(!(*AUX_MU_LSR&0x01));  // wait until something is in the buffer
  
  r=(char)(*AUX_MU_IO);  // read it and return
  
  return r=='\r'?'\n':r;  // convert carrige return to newline
}

void uart_puts(char *s) {
  while(*s) {
    if(*s=='\n')
      uart_send('\r');
    uart_send(*s++);
  }
}


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

char uart_tx_buffer[MAX_BUF_SIZE] = {};
unsigned int uart_tx_buffer_widx = 0; //write index
unsigned int uart_tx_buffer_ridx = 0; //read index
char uart_rx_buffer[MAX_BUF_SIZE] = {};
unsigned int uart_rx_buffer_widx = 0;
unsigned int uart_rx_buffer_ridx = 0;

void uart_interrupt_r_handler(){
  if ((uart_rx_buffer_widx + 1) % MAX_BUF_SIZE == uart_rx_buffer_ridx){
    enable_uart_r_interrupt();
    return;
  }
  // uart_rx_buffer[uart_rx_buffer_widx++] = uart_getc();
  char r = (char)(*AUX_MU_IO);
  uart_rx_buffer[uart_rx_buffer_widx++] = r=='\r'?'\n':r;
  if (uart_rx_buffer_widx >= MAX_BUF_SIZE)
    uart_rx_buffer_widx = 0;
  enable_uart_r_interrupt();
}

void uart_interrupt_w_handler() { //can write
  if (uart_tx_buffer_ridx == uart_tx_buffer_widx){ // buffer empty
    disable_uart_w_interrupt();
    return;
  }
  // uart_send(uart_tx_buffer[uart_tx_buffer_ridx++]);
  *AUX_MU_IO = uart_tx_buffer[uart_tx_buffer_ridx++];
  if (uart_tx_buffer_ridx >= MAX_BUF_SIZE)
    uart_tx_buffer_ridx = 0; // cycle pointer
  enable_uart_w_interrupt();
}

void async_uart_send(char c){
  uart_tx_buffer[uart_tx_buffer_widx++] = c;
  enable_uart_w_interrupt();
  if (uart_tx_buffer_widx >= MAX_BUF_SIZE)
    uart_tx_buffer_widx = 0;
}

void async_uart_puts(char *s){
  while(*s) {
    if(*s=='\n')
      async_uart_send('\r');
    async_uart_send(*s++);
  }
}

char async_uart_getc(){
  if (uart_rx_buffer_ridx == uart_rx_buffer_widx){
    return 0;
  }
  char r = uart_rx_buffer[uart_rx_buffer_ridx++];
  if (uart_rx_buffer_ridx >= MAX_BUF_SIZE)
    uart_rx_buffer_ridx = 0;
  return r;
}

void enable_uart_r_interrupt(){
  *AUX_MU_IER |= (1);  
}
void enable_uart_w_interrupt(){
  *AUX_MU_IER |= (2);
}

void disable_uart_r_interrupt(){
  *AUX_MU_IER &= ~(1);  
}
void disable_uart_w_interrupt(){
  *AUX_MU_IER &= ~(2);  
}

#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"
#include "utils.h"
#include "mini_uart.h"

// The AUX_MU_LSR_REG register (8 bits) shows the data status.
// The AUX_MU_IO_REG register (8 bits) is primary used to write data to and read data from the UART FIFOs. 
void uart_send ( char c )
{
	while(!(*AUX_MU_LSR_REG & 0x20)) {} // Bit 5, if set to 1, tells us that the transmitter is empty.
	*AUX_MU_IO_REG = c;
}

char uart_recv ( void )
{
	while(!(*AUX_MU_LSR_REG & 0x01)) {} // Bit 0, if set to 1, indicates that the data is ready.
	char recv = *AUX_MU_IO_REG & 0xFF;
	return recv != '\r' ? recv : '\n';
}

void uart_recv_string(char *buffer) {
    int size = 0;
    while(size < MAX_BUFFER_SIZE){
        buffer[size] = uart_recv();
        uart_send(buffer[size]);
        if(buffer[size++] == '\n'){
            break;
        }
    }
    buffer[--size] = '\0';
}


void uart_send_string(char* str)
{
	for (int i = 0; str[i] != '\0'; i ++) {
		uart_send((char)str[i]);
	}
}

unsigned char uart_getb(){ //for data transfer
	unsigned char r;
	do{ asm volatile("nop"); } while (!(*AUX_MU_LSR_REG & 0x01));
	r = (unsigned char)(*AUX_MU_IO_REG);
	return r;
}

void delay(unsigned int clock) {
    while (clock--) {
        asm volatile("nop");
    }
}

// An alternative function is a number from 0 to 5 that can be set for each pin and configures which device is connected to the pin.
// GPFSEL1 register is used to control alternative functions for pins 10-19. 
// pin 14 -> TXD1: set bits 14-12 to 5
// pin 15 -> RXD1: set bits 17-15 to 5
void uart_init ( void )
{	
	// connect Mini UART to the GPIO pins
	unsigned int selector;
	selector = *GPFSEL1;
	selector &= ~(7u << 12);             // Clean gpio14
	selector |= 2u << 12;                // Set alt5 for gpio14
	selector &= ~(7u << 15);             // Clean gpio15
	selector |= 2u << 15;                // Set alt5 for gpio15
	*GPFSEL1 = selector;

	// Remove both the pull-up and pull-down states from a pin
	*GPPUD = 0;
	delay(150u);
	*GPPUDCLK0 = (1u << 14) | (1u << 15);
	delay(150u);
	*GPPUDCLK0 = 0;

	// Initializing the Mini UART
	*AUX_ENABLES = 1u;                   // Enable mini uart (this also enables access to its registers)
	*AUX_MU_CNTL_REG = 0u;               // Disable auto flow control and disable receiver and transmitter (for now)
	*AUX_MU_IER_REG = 0u;                // Disable receive and transmit interrupts
	*AUX_MU_LCR_REG = 3u;                // Enable 8 bit mode
	*AUX_MU_MCR_REG = 0u;                // Set RTS line to be always high
	*AUX_MU_BAUD_REG = 270u;             // Set baud rate to 115200

	*AUX_MU_CNTL_REG = 3u;               // Finally, enable transmitter and receiver
}
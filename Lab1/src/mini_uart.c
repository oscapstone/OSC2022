#include "utils.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"

// The AUX_MU_LSR_REG register (8 bits) shows the data status.
// The AUX_MU_IO_REG register (8 bits) is primary used to write data to and read data from the UART FIFOs. 
void uart_send ( char c )
{
	while(1) {
		if(get32(AUX_MU_LSR_REG)&0x20)	// Bit 5, if set to 1, tells us that the transmitter is empty.
			break;
	}
	put32(AUX_MU_IO_REG,c);
}

char uart_recv ( void )
{
	while(1) {
		if(get32(AUX_MU_LSR_REG)&0x01)	// Bit 0, if set to 1, indicates that the data is ready.
			break;
	}
	return(get32(AUX_MU_IO_REG)&0xFF);
}

void uart_send_string(char* str)
{
	for (int i = 0; str[i] != '\0'; i ++) {
		uart_send((char)str[i]);
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
	selector = get32(GPFSEL1);
	selector &= ~(7<<12);                   // Clean gpio14
	selector |= 2<<12;                      // Set alt5 for gpio14
	selector &= ~(7<<15);                   // Clean gpio15
	selector |= 2<<15;                      // Set alt5 for gpio15
	put32(GPFSEL1,selector);

	// Remove both the pull-up and pull-down states from a pin
	put32(GPPUD,0);
	delay(150);
	put32(GPPUDCLK0,(1<<14)|(1<<15));
	delay(150);
	put32(GPPUDCLK0,0);

	// Initializing the Mini UART
	put32(AUX_ENABLES,1);                   // Enable mini uart (this also enables access to its registers)
	put32(AUX_MU_CNTL_REG,0);               // Disable auto flow control and disable receiver and transmitter (for now)
	put32(AUX_MU_IER_REG,0);                // Disable receive and transmit interrupts
	put32(AUX_MU_LCR_REG,3);                // Enable 8 bit mode
	put32(AUX_MU_MCR_REG,0);                // Set RTS line to be always high
	put32(AUX_MU_BAUD_REG,270);             // Set baud rate to 115200

	put32(AUX_MU_CNTL_REG,3);               // Finally, enable transmitter and receiver
}
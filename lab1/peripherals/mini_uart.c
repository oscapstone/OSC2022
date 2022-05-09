#include "types.h"
#include "peripherals/iomapping.h"
#include "peripherals/mini_uart.h"
void delay_cycles(uint64_t n){
    for(register uint64_t i = 0 ; i < n ; i++) asm volatile("nop");
}

/**
 * @brief Enable UART 
 */
void mini_uart_init(void){
    // GPIO 14 & 15 take function 0
    // Read out GPFSEL1 register
    uint32_t tmp = IO_MMIO_read32(GPFSEL1);

    // Set GPIO 14 & 15 function bits
    tmp &= ~(0b111111 << 12);
    tmp |= 0b010010 << 12;
    IO_MMIO_write32(GPFSEL1, tmp);
    

    // disable pull-up/down of GPIO 14 & 15 
    // Write to GPPUD to set the required control signal
    IO_MMIO_write32(GPPUD, 0);

    // Wait 150 cycles – this provides the required set-up time for the control signal 
    delay_cycles(150);

    // Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads you wish to modify
    IO_MMIO_write32(GPPUDCLK0, (1 << 14) | (1 << 15));

    // Wait 150 cycles – this provides the required hold time for the control signal
    delay_cycles(150);

    // Write to GPPUD to remove the control signa
    IO_MMIO_write32(GPPUD, 0);

    // Write to GPPUDCLK0/1 to remove the clock 
    IO_MMIO_write32(GPPUDCLK0, 0);

    // Initialize Auxiliary peripherals Register
    // Mini UART enable
    IO_MMIO_write32(AUX_ENABLES, 1);

    // To disable auto flow control and disable receiver and transmitter, set control register to 0
    IO_MMIO_write32(AUX_MU_CNTL_REG, 0);

    // Disable mini UART's interrupt
    IO_MMIO_write32(AUX_MU_IER_REG, 0);
 
    // Set UART to 8-bit mode
    IO_MMIO_write32(AUX_MU_LCR_REG, 3);

    // To indicate that our UART is able to receive data, set RTS line to be always high
    IO_MMIO_write32(AUX_MU_MCR_REG, 0);

    // Set Baud rate to 115200
    IO_MMIO_write32(AUX_MU_BAUD_REG, BAUD_RATE_REG);
    
    // Start UART
    IO_MMIO_write32(AUX_MU_CNTL_REG, 3);
    return;
}

uint8_t mini_uart_read(void){
    while(!(IO_MMIO_read32(AUX_MU_LSR_REG) & 0x1));
    return IO_MMIO_read32(AUX_MU_IO_REG) & 0xff;
}

void mini_uart_write(uint8_t val){
    while(!(IO_MMIO_read32(AUX_MU_LSR_REG) & (0x1 << 5)));
    IO_MMIO_write32(AUX_MU_IO_REG, val);
}

ssize_t write_bytes(uint8_t* buf, size_t n){
    for(uint64_t i = 0 ; i < n ; i++) mini_uart_write(buf[i]);
    return n;
}

void write_str(char* buf){
    while(*buf) mini_uart_write((uint8_t)*buf++);
    return;
}



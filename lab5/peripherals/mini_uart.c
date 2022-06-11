#include "peripherals/mini_uart.h"

static ring_buffer* rx_rbuf = NULL;

inline void delay_cycles(uint64_t n){
    for(register uint64_t i = 0 ; i < n ; i++) asm volatile("nop");
}

inline void enable_mini_uart_rx_irq(){
}

inline void disable_mini_uart_rx_irq(){
}

void mini_uart_init(){
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
    
    // disable FIFO
    IO_MMIO_write32(AUX_MU_IIR_REG, 6);

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

void write_hex(uint64_t n){
    int i = 0;
    char buf[16];
    char *hex_table = "0123456789abcdef";
    do{
        buf[i] = hex_table[n & 0xf];
        n = n >> 4;
        i++;
    }while(n);
    do{
        i--;
        mini_uart_write(buf[i]);
    }while(i > 0);
}

void mini_uart_irq_init(){
    rx_rbuf = create_simple_ring_buf(4095);

    IO_MMIO_write32(AUX_MU_IER_REG, 1);
    IO_MMIO_write32(ENABLE_IRQS_1, 1 << 29);
}


void mini_uart_rx_softirq_callback(){
}

void mini_uart_irq_read(){
    uint8_t b[1];
    b[0] = IO_MMIO_read32(AUX_MU_IO_REG) & 0xff;
    ring_buf_write_unsafe(rx_rbuf, b, 1);
}

size_t mini_uart_get_rx_len(){
    return ring_buf_get_len(rx_rbuf);
}

uint8_t mini_uart_aio_read(void){
    uint8_t b[1];
    IO_MMIO_write32(AUX_MU_IER_REG, 1);
    while(!ring_buf_read(rx_rbuf, b, 1));
    IO_MMIO_write32(AUX_MU_IER_REG, 0);
    return b[0];
}

size_t sys_uart_write(char *buf, size_t size){
    uint64_t daif,ret;
    //size_t c;
    daif = local_irq_disable_save();
    ret = write_bytes(buf, size);
    local_irq_restore(daif);
    return ret;
}

size_t sys_uart_read(char *buf, size_t size){
    size_t c = 0, tmp;
    IO_MMIO_write32(AUX_MU_IER_REG, 1);
    while(size){
        tmp = ring_buf_read(rx_rbuf, buf + c, size);

        size = size - tmp;
        c = c + tmp;
    }
    IO_MMIO_write32(AUX_MU_IER_REG, 0);
    return c;
}

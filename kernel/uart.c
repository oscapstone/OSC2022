#include "uart.h"

char uart_read_buf[MAX_BUF_SIZE] = {};
char uart_write_buf[MAX_BUF_SIZE] = {};
unsigned int uart_read_buf_begin = 0, uart_read_buf_end = 0;
unsigned int uart_write_buf_begin = 0, uart_write_buf_end = 0;

void uart_init() {
    register unsigned int r;

    /* initialize UART */
    // 1. Set AUXENB register to enable mini UART. Then mini UART register can be accessed.
    *AUX_ENABLES |= 1;
    // 2. Set AUX_MU_CNTL_REG to 0. Disable transmitter and receiver during configuration.
    *AUX_MU_CNTL_REG = 0;
    // 3. Set AUX_MU_IER_REG to 0. Disable interrupt because currently you don’t need interrupt.
    *AUX_MU_IER_REG = 0;
    // 4. Set AUX_MU_LCR_REG to 3. Set the data size to 8 bit.
    *AUX_MU_LCR_REG = 3;
    // 5. Set AUX_MU_MCR_REG to 0. Don’t need auto flow control.
    *AUX_MU_MCR_REG = 0;
    // 6. Set AUX_MU_BAUD to 270. Set baud rate to 115200
    *AUX_MU_BAUD_REG = 270;
    // 7. Set AUX_MU_IIR_REG to 6. No FIFO.
    *AUX_MU_IIR_REG = 6;

    /* map UART1 to GPIO pins */
    /* mini UART  -> set ALT5, PL011 UART -> set ALT0 */
    /* Configure GPFSELn register to change alternate function */
    r = *GPFSEL1;
    r &= ~((7 << 12) | (7 << 15));  // clear gpio14,15 alternate function
    r |= (2 << 12) | (2 << 15);     // alternate function 5 (TXD1 and RXD1)
    *GPFSEL1 = r;

    /* Enable pins 14 and 15 (disable pull-up/down, "floating" input pin with no pull-up or pull-down resistors) */
    *GPPUD = 0;
    /* Wait 150 cycles – this provides the required set-up time for the control signal */
    nop_delay(150);
    /* Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads you wish to modify */
    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    /* Wait 150 cycles – this provides the required set-up time for the control signal */
    nop_delay(150);
    /* flush GPIO setup */
    *GPPUDCLK0 = 0;

    // 8. Set AUX_MU_CNTL_REG to 3. Enable the transmitter and receiver.
    *AUX_MU_CNTL_REG = 3;

    // clear rx data
    while ((*AUX_MU_LSR_REG & 0x01))
        *AUX_MU_IO_REG;
}

void uart_write_char(char c) {
    do{asm volatile("nop");}while(!(*AUX_MU_LSR_REG & 0x20));
    *AUX_MU_IO_REG = (unsigned int)c;
}

void uart_write_string(char* str) {
    for (unsigned int i = 0; str[i] != '\0'; i++) {
        uart_write_char((char)str[i]);
    }
}

void uart_write_hex(unsigned int d) {
    unsigned int c;
    for (int i = 28; i >= 0; i -= 4) {
        /* Highest 4 bits */
        c = (d >> i) & 0xF;
        /* Translate to hex */
        c = (c > 9) ? (0x37 + c) : (0x30 + c);
        uart_write_char(c);
    }
}

int uart_printf(char *fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    char buf[MAX_BUF_SIZE];
    char *s = (char *)buf;
    int count = vsprintf(s, fmt, args);
    while (*s)
        uart_write_char(*s++);
    __builtin_va_end(args);
    return count;
}

void uart_read(char* buf, unsigned int size) {
    for (unsigned int i = 0; i < size; i++) {
        do{asm volatile("nop");}while(!(*AUX_MU_LSR_REG & 0x01));
        buf[i] = (char)(*AUX_MU_IO_REG);
    }
}

void nop_delay(unsigned int t) {
    for (unsigned int i = 0; i < t; i++)
        asm volatile("nop");
}

/*  Interrupt   */

// read data to read buffer
void uart_interrupt_r_handler() {
    // read buffer full => disable read interrupt
    if ((uart_read_buf_end + 1) % MAX_BUF_SIZE == uart_read_buf_begin) {
        // NOT SURE == clear fifo
        *AUX_MU_IIR_REG = 0xc2;
        disable_mini_uart_r_interrupt();
        return;
    }
    // read to buffer
    uart_read(&uart_read_buf[uart_read_buf_end++], 1);
    // circular buffer
    if (uart_read_buf_end >= MAX_BUF_SIZE)
        uart_read_buf_end = 0;
    
    // unmasks the device’s interrupt line
    enable_mini_uart_r_interrupt();
}

// write data from write buffer
void uart_interrupt_w_handler() {
    // buffer is empty => disable write interrupt
    if (uart_write_buf_begin == uart_write_buf_end) {
        *AUX_MU_IIR_REG = 0xc4;
        disable_mini_uart_w_interrupt();
        return;
    }
    // write from buffer
    uart_write_char(uart_write_buf[uart_write_buf_begin++]);
    // circular buffer
    if (uart_write_buf_begin >= MAX_BUF_SIZE)
        uart_write_buf_begin = 0;

    // unmasks the device’s interrupt line
    enable_mini_uart_w_interrupt();
}

void enable_mini_uart_interrupt() {
    enable_mini_uart_r_interrupt();
    enable_mini_uart_w_interrupt();
    *IRQS1 |= 1 << 29;
}

void disable_mini_uart_interrupt() {
    disable_mini_uart_r_interrupt();
    disable_mini_uart_w_interrupt();
}

void enable_mini_uart_r_interrupt() {
    *AUX_MU_IER_REG |= 1;
}

void enable_mini_uart_w_interrupt() {
    *AUX_MU_IER_REG |= 2;
}

void disable_mini_uart_r_interrupt() {
    *AUX_MU_IER_REG &= ~(1);
}

void disable_mini_uart_w_interrupt() {
    *AUX_MU_IER_REG &= ~(2);
}

int mini_uart_r_interrupt_is_enable() {
    return *AUX_MU_IER_REG & 1;
}

int mini_uart_w_interrupt_is_enable() {
    return *AUX_MU_IER_REG & 2;
}

/*  Async   */

// wrtie data to write buffer
void uart_write_char_async(char c) {
    // wait for full buffer
    while ((uart_write_buf_end + 1) % MAX_BUF_SIZE == uart_write_buf_begin) {
        // start asynchronous transfer
        enable_mini_uart_w_interrupt();
    }
    // critical section
    lock();
    // write to buffer
    uart_write_buf[uart_write_buf_end++] = c;
    // circular buffer
    if (uart_write_buf_end >= MAX_BUF_SIZE)
        uart_write_buf_end = 0;
    // start asynchronous transfer
    unlock();
    // enable interrupt to transfer
    enable_mini_uart_w_interrupt();
}

void uart_write_string_async(char* str) {
    for (unsigned int i = 0; str[i] != '\0'; i++) {
        uart_write_char_async((char)str[i]);
    }
}

void uart_write_hex_async(unsigned int d) {
    unsigned int c;
    for (int i = 28; i >= 0; i -= 4) {
        /* Highest 4 bits */
        c = (d >> i) & 0xF;
        /* Translate to hex */
        c = (c > 9) ? (0x37 + c) : (0x30 + c);
        uart_write_char_async(c);
    }
}

int uart_printf_async(char *fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    char buf[MAX_BUF_SIZE];
    // we don't have memory allocation yet, so we
    // simply place our string after our code
    char *s = (char *)buf;
    // use sprintf to format our string
    int count = vsprintf(s, fmt, args);
    // print out as usual
    while (*s)
        uart_write_char_async(*s++);
    __builtin_va_end(args);
    return count;
}

// read data from read buffer
char uart_read_char_async() {
    enable_mini_uart_r_interrupt();
    // while buffer empty => enable read interrupt to get some input into buffer
    while (uart_read_buf_begin == uart_read_buf_end)
        enable_mini_uart_r_interrupt();
    
    // critical section
    lock();
    char r = uart_read_buf[uart_read_buf_begin++];

    if (uart_read_buf_begin >= MAX_BUF_SIZE)
        uart_read_buf_begin = 0;
    
    unlock();
    return r;
}

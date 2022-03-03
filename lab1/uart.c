#include "gpio.h"
#include "myprint.h"
#include "mailbox.h"

#define UART0_BASE      (MMIO_BASE + 0x201000)

#define UART0_DR        ((unsigned int*)(UART0_BASE))
#define UART0_FR        ((unsigned int*)(UART0_BASE + 0x18))
#define UART0_IBRD      ((unsigned int*)(UART0_BASE + 0x24))
#define UART0_FBRD      ((unsigned int*)(UART0_BASE + 0x28))
#define UART0_LCRH      ((unsigned int*)(UART0_BASE + 0x2C))
#define UART0_CR        ((unsigned int*)(UART0_BASE + 0x30))
#define UART0_IMSC      ((unsigned int*)(UART0_BASE + 0x38))
#define UART0_ICR       ((unsigned int*)(UART0_BASE + 0x44))

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

#define MBOX_BASE                       (MMIO_BASE + 0xb880)

// address map
#define MBOX_READ                       (unsigned int*)(MBOX_BASE)
#define MBOX_STATUS                     (unsigned int*)(MBOX_BASE + 0x18)
#define MBOX_WRITE                      (unsigned int*)(MBOX_BASE + 0x20)

// flag
#define MBOX_EMPTY                      0x40000000
#define MBOX_FULL                       0x80000000

// code
#define MBOX_CODE_BUF_REQ               0x00000000
#define MBOX_CODE_BUF_RES_SUCC          0x80000000
#define MBOX_CODE_TAG_REQ               0x00000000

// tag
#define MBOX_TAG_GET_BOARD_REVISION     0x00010002
#define MBOX_TAG_GET_VC_MEMORY          0x00010006
#define MBOX_TAG_SET_CLOCK_RATE         0x00038002
#define MBOX_TAG_SET_PHY_WIDTH_HEIGHT   0x00048003
#define MBOX_TAG_SET_VTL_WIDTH_HEIGHT   0x00048004
#define MBOX_TAG_SET_VTL_OFFSET         0x00048009
#define MBOX_TAG_SET_DEPTH              0x00048005
#define MBOX_TAG_SET_PIXEL_ORDER        0x00048006
#define MBOX_TAG_ALLOCATE_BUFFER        0x00040001
#define MBOX_TAG_GET_PITCH              0x00040008



void uart_init(){
    *UART0_CR = 0;  // turn off UART0

    /* Initialize UART */
    *AUX_ENABLE |= 1;   // Enable mini UART
    *AUX_MU_CNTL = 0;    // Disable TX, RX during configuration
    *AUX_MU_IER = 0;     // Disable interrupt
    *AUX_MU_LCR = 3;     // Set the data size to 8 bit
    *AUX_MU_MCR = 0;     // Don't need auto flow control
    *AUX_MU_BAUD = 270;  // Set baud rate to 115200
    *AUX_MU_IIR = 6;     // No FIFO

    /* Configure UART0 Clock Frequency */
    unsigned int __attribute__((aligned(16))) mbox[9];
    mbox[0] = 9 * 4;
    mbox[1] = MBOX_CODE_BUF_REQ;
    // tags begin
    mbox[2] = MBOX_TAG_SET_CLOCK_RATE;
    mbox[3] = 12;
    mbox[4] = MBOX_CODE_TAG_REQ;
    mbox[5] = 2;        // UART clock
    mbox[6] = 4000000;  // 4MHz
    mbox[7] = 0;        // clear turbo
    mbox[8] = 0x0;      // end tag
    // tags end
    mbox_call(mbox, 8);

    /* Map UART to GPIO Pins */

    // 1. Change GPIO 14, 15 to alternate function
    register unsigned int r = *GPFSEL1;
    r &= ~((7 << 12) | (7 << 15));  // Reset GPIO 14, 15
    r |= (2 << 12) | (2 << 15);     // Set ALT5
    *GPFSEL1 = r;

    // 2. Disable GPIO pull up/down (Because these GPIO pins use alternate functions, not basic input-output)
    // Set control signal to disable
    *GPPUD = 0;
    // Wait 150 cycles
    r = 150;
    while (r--) {
        asm volatile("nop");
    }
    // Clock the control signal into the GPIO pads
    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    // Wait 150 cycles
    r = 150;
    while (r--) {
        asm volatile("nop");
    }
    // Remove the clock
    *GPPUDCLK0 = 0;

    // 3. Enable TX, RX
    *AUX_MU_CNTL = 3;

    /* Configure UART0 */
    *UART0_IBRD = 0x2;        // Set 115200 Baud
    *UART0_FBRD = 0xB;        // Set 115200 Baud
    *UART0_LCRH = 0b11 << 5;  // Set word length to 8-bits
    *UART0_ICR = 0x7FF;       // Clear Interrupts

    /* Enable UART */
    *UART0_CR = 0x301;

}


/**
 * Send a character
 */
void uart_send(unsigned int c){
    /* wait until we can send */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
    /* write the character to the buffer */
    *AUX_MU_IO=c;
}

/**
 * Receive a character
 */
char uart_getc(){
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


char uart_read(){
    // Check data ready field
    do {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR & 0x01));
    // Read
    char r = (char)(*AUX_MU_IO);
    // Convert carrige return to newline
    return r == '\r' ? '\n' : r;
}

void uart_write(unsigned int c){
    // Check transmitter idle field
    do {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR & 0x20));
    // Write
    *AUX_MU_IO = c;
}

void uart_printf(char* fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    extern volatile unsigned char _end;  // defined in linker
    char* s = (char*)&_end;              // put temporary string after code
    vsprintf(s, fmt, args);

    while (*s) {
        if (*s == '\n') uart_write('\r');
        uart_write(*s++);
    }
}

void uart_flush(){
    while (*AUX_MU_LSR & 0x01) {
        *AUX_MU_IO;
    }
}


#include "../include/UART.h"

#define physical_add     0x3F000000

// initialization
#define AUXENB      ((volatile unsigned int*)(physical_add+0x00215004))
#define AUX_MU_CNTL_REG     ((volatile unsigned int*)(physical_add+0x00215060))
#define AUX_MU_IER_REG      ((volatile unsigned int*)(physical_add+0x00215044))
#define AUX_MU_LCR_REG      ((volatile unsigned int*)(physical_add+0x0021504C))
#define AUX_MU_MCR_REG      ((volatile unsigned int*)(physical_add+0x00215050))
#define AUX_MU_BAUD     ((volatile unsigned int*)(physical_add+0x00215068))
#define AUX_MU_IIR_REG      ((volatile unsigned int*)(physical_add+0x00215048))

// Read,write data
#define AUX_MU_LSR_REG      ((volatile unsigned int*)(physical_add+0x00215054))
#define AUX_MU_IO_REG       ((volatile unsigned int*)(physical_add+0x00215040))

#define AUX_MU_MSR      ((volatile unsigned int*)(physical_add+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(physical_add+0x0021505C))
#define AUX_MU_STAT     ((volatile unsigned int*)(physical_add+0x00215064))

#define GPFSEL0         ((volatile unsigned int*)(physical_add+0x00200000))
#define GPFSEL1         ((volatile unsigned int*)(physical_add+0x00200004))
#define GPFSEL2         ((volatile unsigned int*)(physical_add+0x00200008))
#define GPFSEL3         ((volatile unsigned int*)(physical_add+0x0020000C))
#define GPFSEL4         ((volatile unsigned int*)(physical_add+0x00200010))
#define GPFSEL5         ((volatile unsigned int*)(physical_add+0x00200014))
#define GPSET0          ((volatile unsigned int*)(physical_add+0x0020001C))
#define GPSET1          ((volatile unsigned int*)(physical_add+0x00200020))
#define GPCLR0          ((volatile unsigned int*)(physical_add+0x00200028))
#define GPLEV0          ((volatile unsigned int*)(physical_add+0x00200034))
#define GPLEV1          ((volatile unsigned int*)(physical_add+0x00200038))
#define GPEDS0          ((volatile unsigned int*)(physical_add+0x00200040))
#define GPEDS1          ((volatile unsigned int*)(physical_add+0x00200044))
#define GPHEN0          ((volatile unsigned int*)(physical_add+0x00200064))
#define GPHEN1          ((volatile unsigned int*)(physical_add+0x00200068))
#define GPPUD           ((volatile unsigned int*)(physical_add+0x00200094))
#define GPPUDCLK0       ((volatile unsigned int*)(physical_add+0x00200098))
#define GPPUDCLK1       ((volatile unsigned int*)(physical_add+0x0020009C))

void UART_init(){
    *AUXENB |= 1; 
    *AUX_MU_CNTL_REG = 0;
    *AUX_MU_IER_REG = 0;  
    *AUX_MU_LCR_REG = 3;
    *AUX_MU_MCR_REG = 0;
    *AUX_MU_BAUD = 270;
    *AUX_MU_IIR_REG = 0xc6;  

    register unsigned int r = *GPFSEL1;  // for gpio pin 10-19
    r &= ~((7<<12)|(7<<15)); // gpio14, gpio15
    r |= (2<<12)|(2<<15);    // alt5
    *GPFSEL1 = r;

    *GPPUD = 0;    // disable gpio pull up/down
    r=150; 
    while(r--)
        asm volatile("nop");
    *GPPUDCLK0 = (1<<14)|(1<<15);  // for gpio pin 14,15
    r=150; 
    while(r--) 
        asm volatile("nop");
    *GPPUDCLK0 = 0;        // flush GPIO setup
    *AUX_MU_CNTL_REG = 3;     
}

char UART_read(){
    do{
        asm volatile("nop");
    } while(!(*AUX_MU_LSR_REG & 0x01));  // check ready filed

    char c = (char)(*AUX_MU_IO_REG);
    return c=='\r'?'\n':c;
}

void UART_write(unsigned int c){
    do{
        asm volatile("nop");
    } while(!(*AUX_MU_LSR_REG & 0x20));  // check transmitter empty filed

    *AUX_MU_IO_REG = c;
}

void UART_put(char *s){
    while(*s){
        if(*s == '\n')
            UART_write('\r');
        UART_write(*s++);
    }
}

void UART_hex(unsigned int c){
    for(int i=28;i>=0;i-=4){
        unsigned int n = (c>>i)&0XF;    // 4 bits from left to right

        n += n>9?0X37:0X30;         // 0-9->'0'-'9', 10-15->'A'->'F';
        UART_write(n);
    }
}




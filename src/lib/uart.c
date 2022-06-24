#include <stdint.h>
#include <stddef.h>
#include <uart.h>
#include <mmio.h>
#include <string.h>
#include <interrupt.h>
#include <ringbuffer.h>
#include <sched.h>
#include <queue.h>
#include <lock.h>
#define RECEIVER_ENABLE_BIT (1<<0)
#define TRANSMITTER_ENABLE_BIT (1<<1)
#define NEWLINE '\n'
#define NEWLINE_OUT "\n\r"
#define UART_BUF_LEN 0x200

RingBuffer *uart_buffer;
Queue *uart_read_waitqueue;
Lock *uart_read_lock;

void uart_init()
{
    //*GPFSEL1 &= 0xfffc0fff;
    //*GPFSEL1 |= 0b010010<<12;
    
    mmio_set(AUX_ENABLES, 1);
    //mmio_set(AUX_MU_CNTL_REG, 0xfffffffe);
    mmio_set(AUX_MU_CNTL_REG, 0b0);
    mmio_set(AUX_MU_LCR_REG, 3);
    mmio_set(AUX_MU_MCR_REG, 0);
    mmio_set(AUX_MU_BAUD_REG, 270);
    mmio_set(AUX_MU_IIR_REG, 0xc6);
    mmio_set(GPFSEL1, (mmio_load(GPFSEL1)&0xfffc0fff)|(0b010010<<12));
    mmio_set(GPPUD, 0);
    unsigned int r;
    r=150; while(r--) { asm volatile("nop"); }
    mmio_set(GPPUDCLK0, (1<<14)|(1<<15));
    r=150; while(r--) { asm volatile("nop"); }
    mmio_set(GPPUDCLK0, 0);        // flush GPIO setup
    mmio_set(AUX_MU_CNTL_REG, 3);
    
    // enable interrupt
    //mmio_set(AUX_MU_IER_REG, 0b0);
    mmio_set(AUX_MU_IER_REG, 0b01);
    mmio_set(ARMINT_En_IRQs1_REG, mmio_load(ARMINT_En_IRQs1_REG) | (1<<29));

    uart_buffer = RingBuffer_new(UART_BUF_LEN);
    uart_read_waitqueue = queue_new();
    uart_read_lock = lock_new();
}

void uart_interrupt_handler()
{
    interrupt_disable();
    while((mmio_load(AUX_MU_LSR_REG) & 1) && !RingBuffer_Full(uart_buffer)){
        RingBuffer_writeb(uart_buffer, mmio_load(AUX_MU_IO_REG)&0xff);
    }
    wakeup(uart_read_waitqueue);
    // uart_putshex(uart_buffer->len);
    // uart_putshex(uart_buffer->lbound);
    // uart_putshex(uart_buffer->rbound);
    // uart_puts(uart_buffer->buf);
    interrupt_enable();
}

size_t uart_read_sync(char* buf, size_t len)
{
    //*AUX_MU_CNTL_REG = 0xffffffff & RECEIVER_ENABLE_BIT;
    //unsigned int oldcntl = mmio_load(AUX_MU_CNTL_REG);
    //mmio_set(AUX_MU_CNTL_REG, (oldcntl&0b00)|RECEIVER_ENABLE_BIT);
    size_t recvlen = 0;
    while(recvlen < len){
        while(!(mmio_load(AUX_MU_LSR_REG) & 1));
        buf[recvlen++] = (char)(mmio_load(AUX_MU_IO_REG)&0xff);
        //if(buf[recvlen-1]!=NEWLINE)uart_write(&buf[recvlen-1], 1);
        //else uart_write(NEWLINE_OUT, 2);
    }
    //*AUX_MU_CNTL_REG &= ~RECEIVER_ENABLE_BIT;
    //mmio_set(AUX_MU_CNTL_REG, oldcntl);
    return recvlen;
}

size_t uart_read_async(char* buf, size_t len)
{
    lock_get(uart_read_lock);
    size_t recvlen = 0;
    while(recvlen < len){
        while(RingBuffer_Empty(uart_buffer))wait(uart_read_waitqueue);
        if(RingBuffer_readb(uart_buffer, &buf[recvlen]) == 1) recvlen++;
        //uart_puts("recv");
    }
    lock_release(uart_read_lock);
    return recvlen;
}

size_t uart_write(const char* buf, size_t len)
{
    //*AUX_MU_CNTL_REG = 0xffffffff & TRANSMITTER_ENABLE_BIT;
    //unsigned int oldcntl = mmio_load(AUX_MU_CNTL_REG);
    //mmio_set(AUX_MU_CNTL_REG, (oldcntl&0b00)|TRANSMITTER_ENABLE_BIT);
    size_t writelen = 0;
    while(writelen < len){
        while(!(mmio_load(AUX_MU_LSR_REG) & 0b100000));
        mmio_set(AUX_MU_IO_REG, (mmio_load(AUX_MU_IO_REG)&0xffffff00) | buf[writelen++]);
        //*AUX_MU_IO_REG |= buf[writelen++];
    }
    //*AUX_MU_CNTL_REG &= ~TRANSMITTER_ENABLE_BIT;
    //mmio_set(AUX_MU_CNTL_REG, oldcntl);
    return writelen;
}

void uart_print(const char* buf)
{
    int i=0;
    while(buf[i]){
        uart_write(&buf[i], 1);
        i++;
    }
}

void uart_puts(const char* buf)
{
    uart_print(buf);
    uart_write(NEWLINE_OUT, 2);
}

size_t uart_gets(char* buf)
{
    int i=0;
    while(uart_read(&buf[i], 1)){
        if(buf[i]==NEWLINE){
            buf[i] = 0;
            break;
        }
        i++;
    }
    return i;
}


void uart_print_hex(uint64_t num)
{
    char buf[0x15];
    u642hex(num, buf, 0x15);
    uart_print(buf);
}

void uart_print_dec(uint64_t num)
{
    char buf[0x20];
    u642dec(num, buf, 0x20);
    uart_print(buf);
}

void uart_putshex(uint64_t num)
{
    uart_print_hex(num);
    uart_puts("");
}

void uart_puts_dec(uint64_t num)
{
    uart_print_dec(num);
    uart_puts("");
}
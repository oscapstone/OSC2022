#ifndef __EXCEPTION__H__
#define __EXCEPTION__H__
#include "uart.h"
#include "utils.h"
#include "timer.h"
#include "stddef.h"


/* Interrupt Registers */
#define IRQs_1_ENABLE        ((volatile unsigned int*)(MMIO_BASE+0x0000b210))
#define IRQs_1_DISABLE       ((volatile unsigned int*)(MMIO_BASE+0x0000b21c))
#define CORE_0_IRQ_SOURCE	 ((volatile unsigned int *)(0x40000060))
#define IRQ_1_PENDING		 ((volatile unsigned int*)(MMIO_BASE+0x0000b204))

// extern char read_buffer[READ_BUF_SIZE];
// extern char write_buffer[WRITE_BUF_SIZE];

// extern unsigned int read_head;
// extern unsigned int write_head;
// extern unsigned int read_tail;
// extern unsigned int write_tail;


void enable_interrupt ();
void disable_interrupt ();
void set_next_timeout(unsigned int seconds);
void syn_handler();
void irq_handler();
void undefined_handler();
void timer_irq_handler();

#endif
#ifndef __EXCEPTION__H__
#define __EXCEPTION__H__
#include "uart.h"
#include "utils.h"
#include "timer.h"
#include "stddef.h"
#include "schedule.h"

/* Interrupt Registers */
#define IRQs_1_ENABLE        ((volatile unsigned int*)(MMIO_BASE+0x0000b210))
#define IRQs_1_DISABLE       ((volatile unsigned int*)(MMIO_BASE+0x0000b21c))
#define CORE_0_IRQ_SOURCE	 ((volatile unsigned int *)(0x40000060))
#define IRQ_1_PENDING		 ((volatile unsigned int*)(MMIO_BASE+0x0000b204))

void enable_interrupt ();
void disable_interrupt ();
void set_next_timeout(unsigned int seconds);
void syn_handler();
void irq_handler();
void undefined_handler();
void timer_irq_handler();

typedef struct trapframe{
    unsigned long x[31];
    unsigned long sp_el0;
    unsigned long elr_el1;
    unsigned long spsr_el1;
} trapframe_t;

void lower_syn_handler(unsigned long esr_el1, unsigned long elr_el1, trapframe_t *tf);

// void lower_syn_handler(unsigned long spsr_el1, unsigned long elr_el1, unsigned long esr_el1, trapframe_t *tf);

#endif
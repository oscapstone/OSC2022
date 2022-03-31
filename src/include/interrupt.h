#ifndef _DEF_INTERRUPT
#define _DEF_INTERRUPT

#include <stdint.h>
#include <mmio.h>

void interrupt_irq_handler();
void interrupt_fiq_handler();
void interrupt_enable();
void interrupt_disable();

#define ARMINTERRUPT_BASE MMIO_BASE + 0xB000
#define ARMINT_IRQ_PEND_BASE_REG ARMINTERRUPT_BASE + 0x200
#define ARMINT_IRQ_PEND1_REG ARMINTERRUPT_BASE + 0x204
#define ARMINT_IRQ_PEND2_REG ARMINTERRUPT_BASE + 0x208
#define ARMINT_En_IRQs1_REG ARMINTERRUPT_BASE + 0x210

#define CORE0_INTERRUPT_SOURCE 0x40000060

#endif
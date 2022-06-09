#ifndef EXCEPTION_H
#define EXCEPTION_H
#include "tpf.h"
#include "esr.h"
#include "mmu.h"
#include "uart.h"
#include "task.h"
#include "timer.h"
#include "sched.h"
#include "syscall.h"

// https://github.com/Tekki/raspberrypi-documentation/blob/master/hardware/raspberrypi/bcm2836/QA7_rev3.4.pdf (p.16)
#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int*)(PHYS_TO_VIRT(0x40000060)))
#define INTERRUPT_SOURCE_GPU (1<<8)
#define INTERRUPT_SOURCE_CNTPNSIRQ (1<<1)

/*
The basic pending register shows which interrupt are pending. To speed up interrupts processing, a
number of 'normal' interrupt status bits have been added to this register. This makes the 'IRQ
pending base' register different from the other 'base' interrupt registers
*/
// https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf (p.112~115)
#define PBASE PHYS_TO_VIRT(0x3F000000)
#define IRQ_BASIC_PENDING	((volatile unsigned int*)(PBASE+0x0000B200))
#define IRQ_PENDING_1		((volatile unsigned int*)(PBASE+0x0000B204))
#define IRQ_PENDING_2		((volatile unsigned int*)(PBASE+0x0000B208))
#define FIQ_CONTROL		    ((volatile unsigned int*)(PBASE+0x0000B20C))
#define ENABLE_IRQS_1		((volatile unsigned int*)(PBASE+0x0000B210))
#define ENABLE_IRQS_2		((volatile unsigned int*)(PBASE+0x0000B214))
#define ENABLE_BASIC_IRQS	((volatile unsigned int*)(PBASE+0x0000B218))
#define DISABLE_IRQS_1		((volatile unsigned int*)(PBASE+0x0000B21C))
#define DISABLE_IRQS_2		((volatile unsigned int*)(PBASE+0x0000B220))
#define DISABLE_BASIC_IRQS	((volatile unsigned int*)(PBASE+0x0000B224))
#define IRQ_PENDING_1_AUX_INT (1<<29)

void sync_64_router(trapframe_t *tpf, unsigned long x1);
void irq_router(trapframe_t *tpf);
void invalid_exception_router(unsigned long long x0);

static inline void enable_interrupt() {
    asm volatile("msr daifclr, 0xf");
}
static inline void disable_interrupt() {
    asm volatile("msr daifset, 0xf");
}

unsigned long long is_disable_interrupt();
void lock();
void unlock();

#endif

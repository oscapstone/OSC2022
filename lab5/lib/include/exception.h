#include "gpio.h"
#include "stdint.h"

// https://github.com/Tekki/raspberrypi-documentation/blob/master/hardware/raspberrypi/bcm2836/QA7_rev3.4.pdf p16
#define CORE0_INTERRUPT_SOURCE ((volatile uint64_t *)(0x40000060))
#define IRQS1_PENDING   ((volatile uint64_t *)(MMIO_BASE+0x0000b204))
#define INTERRUPT_SOURCE_CNTPNSIRQ (1<<1)

void invalid_exception_router(uint64_t x0);
void irq_router(uint64_t x0);
void sync_excep(uint64_t x0, uint64_t x1);



typedef struct trap_frame{
  uint64_t x0;  uint64_t x1;
  uint64_t x2;  uint64_t x3;
  uint64_t x4;  uint64_t x5;
  uint64_t x6;  uint64_t x7;
  uint64_t x8;  uint64_t x9;
  uint64_t x10; uint64_t x11;
  uint64_t x12; uint64_t x13;
  uint64_t x14; uint64_t x15;
  uint64_t x16; uint64_t x17;
  uint64_t x18; uint64_t x19;
  uint64_t x20; uint64_t x21;
  uint64_t x22; uint64_t x23;
  uint64_t x24; uint64_t x25;
  uint64_t x26; uint64_t x27;
  uint64_t x28; uint64_t x29;
  uint64_t x30; uint64_t pad1;
} trap_frame;

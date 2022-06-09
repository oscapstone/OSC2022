#ifndef RESET_H
#define RESET_H
#include "mmu.h"

#define PM_PASSWORD 0x5a000000
#define PM_RSTC PHYS_TO_VIRT(0x3f10001c)
#define PM_WDOG PHYS_TO_VIRT(0x3f100024)

void set(long addr, unsigned int value);
void reset(int tick);
void cancel_reset();

#endif
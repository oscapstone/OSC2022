#ifndef RESET_H
#define RESET_H

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3f10001c
#define PM_WDOG 0x3f100024

void set(long addr, unsigned int value);
void reset(int tick);
void cancel_reset();

#endif
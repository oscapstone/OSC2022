#ifndef REBOOT_H_
#define REBOOT_H_

#define PM_PASSWORD 0x5a000000
#define PM_RSTC ((volatile unsigned int*)(0x3F10001c))
#define PM_WDOG ((volatile unsigned int*)(0x3F100024))

void set(long, unsigned int);
void reset(int);
void cancel_reset();

#endif
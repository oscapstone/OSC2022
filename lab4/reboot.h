#ifndef REBOOT_H
#define REBOOT_H

#define REBOOT_PM_PASSWORD 0x5a000000
#define REBOOT_PM_RSTC 0x3F10001c
#define REBOOT_PM_WDOG 0x3F100024

void set(long addr, unsigned int value) ;
void reset(int tick) ;
void cancel_reset() ;

#endif
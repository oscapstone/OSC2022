#include "reboot.h"
#include "uart.h"
void set(long addr, unsigned int value) {
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void reset(int tick) {                 // reboot after watchdog timer expire
    set(REBOOT_PM_RSTC, REBOOT_PM_PASSWORD | 0x20);  // full reset
    set(REBOOT_PM_WDOG, REBOOT_PM_PASSWORD | tick);  // number of watchdog tick
}

void cancel_reset() {
    set(REBOOT_PM_RSTC, REBOOT_PM_PASSWORD | 0);  // full reset
    set(REBOOT_PM_WDOG, REBOOT_PM_PASSWORD | 0);  // number of watchdog tick
}
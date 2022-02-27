#include "bcm2357.h"
#include "reboot.h"

void reset(int tick) {
    *PM_RSTC = PM_PASSWORD|0x20;
    *PM_WDOG = PM_PASSWORD|tick;
}


void cancel_reset() {
    *PM_RSTC = PM_PASSWORD | 0;
    *PM_WDOG = PM_PASSWORD | 0;
}
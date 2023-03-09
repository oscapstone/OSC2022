#ifndef _REBOOT_H_
#define _REBOOT_H_

#include "peripherals/iomapping.h"

#define PM_WDOG_RESET               0000000000
#define PM_PASSWORD                 (IO_BASE + 0x1b000000)
#define PM_WDOG_TIME_SET            0x000fffff
#define PM_RSTC_WRCFG_CLR           0xffffffcf
#define PM_RSTC_WRCFG_SET           0x00000030
#define PM_RSTC_WRCFG_FULL_RESET    0x00000020
#define PM_RSTC_RESET               0x00000102

extern void reboot(int tick);

#endif

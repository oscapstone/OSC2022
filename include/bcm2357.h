#ifndef BCM2357_H
#define BCM2357_H

#ifndef WITH_STDLIB
#include "type.h"
#else
#include <stdint.h>
#endif

#define MMIO_BASE  0x3f000000
#define VCMMU_BASE 0xc0000000

#define _virt_addr(addr)                ((volatile uint32_t*)(MMIO_BASE + addr))
#define _virt_addr_abs(base, off)      ((volatile uint32_t*)(base + off))



#define PM_PASSWORD         0x5a000000
#define PM_RSTC             _virt_addr(0x10001c)
#define PM_WDOG             _virt_addr(0x100024)





#endif

#include "mmio.h"
#define PM_PASSWORD 0x5A000000
#define PM_RSTC     (MMIO_BASE_ADDRESS + 0x0010001C)
#define PM_WDOG     (MMIO_BASE_ADDRESS + 0x00100024)

void reset(int tick);
void cancel_reset();
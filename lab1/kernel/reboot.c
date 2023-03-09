#include "types.h"
#include "peripherals/iomapping.h"
#include "kernel/reboot.h"

void reboot(int ticks){
    unsigned int pm_rstc,pm_wdog;
    pm_rstc = IO_MMIO_read32(PM_RSTC);
    
    pm_wdog = PM_PASSWORD | (ticks & PM_WDOG_TIME_SET); 
    pm_rstc = PM_PASSWORD | (pm_rstc & PM_RSTC_WRCFG_CLR) | PM_RSTC_WRCFG_FULL_RESET;

    IO_MMIO_write32(PM_RSTC, pm_rstc);
    IO_MMIO_write32(PM_WDOG, pm_wdog);
}

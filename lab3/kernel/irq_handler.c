#include "peripherals/iomapping.h"
#include "kernel/timer.h"
#include "utils.h"
#include "types.h"
#include "debug/debug.h"
#include "asm.h"
void irq_handelr(){
    uint32_t irq_pending_1 = IO_MMIO_read32(IRQ_PENDING_1);
    uint32_t core0_irq_source = IO_MMIO_read32(CORE0_INTERRUPT_SOURCE);
    if(core0_irq_source & 2){
        core_timer_irq_handler();
    }else{
        LOG("Unkown interrupt, DAIF: %x", get_DAIF());
    }
}

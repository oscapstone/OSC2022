#include "exception.h"

void irq_handler() {
    uint64_t c0_int_src = mmio_read(CORE0_IRQ_SRC),
             irq_pend_1 = mmio_read(IRQ_PEND_1);
    // async_printf("IRQ_PEND_1: 0x%lx" ENDL, mmio_read(IRQ_PEND_1));
    // async_printf("CORE0_IRQ_SRC: 0x%lx" ENDL, c0_int_src);

    // 1. masks the device’s interrupt line,
    disable_interrupt();

    // 2. move data from the device’s buffer through DMA, or manually copy,
    // TODO: necessary???

    // 3. enqueues the processing task to the event queue,
    if (c0_int_src & SRC_CNTPNSIRQ_INT) {
        core_timer_handler();
    } else {  // TODO: better condition
        uart_int_handler();
    }

    // 4. do the tasks with interrupts enabled,

    // 5. unmasks the interrupt line to get the next interrupt at the end of the task.
    enable_interrupt();
}

void invalid_handler(uint32_t x0) {
    printf("[Invalid Exception] %d" ENDL, x0);
}

void sync_handler() {
    printf("sync_handler()" ENDL);
}

void enable_interrupt() {
    asm volatile("msr DAIFClr, 0xf");
}

void disable_interrupt() {
    asm volatile("msr DAIFSet, 0xf");
}
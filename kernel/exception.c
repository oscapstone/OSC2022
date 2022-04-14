#include "exception.h"

void sync64_router(unsigned long long x0) {
    unsigned long long spsr_el1, elr_el1, esr_el1;
    asm volatile("mrs %0, spsr_el1\n\t" : "=r" (spsr_el1) :: "memory");
    asm volatile("mrs %0, elr_el1\n\t" : "=r" (elr_el1) :: "memory");
    asm volatile("mrs %0, esr_el1\n\t" : "=r" (esr_el1) :: "memory");
    uart_printf("Exception : sync_el0_64, SPSR_EL1 : 0x%x, ELR_EL1 : 0x%x, ESR_EL1 : 0x%x\r\n", spsr_el1, elr_el1, esr_el1);
}

void irq_router(unsigned long long x0) {
    // from core timer (CNTPNS) IRQ_EL0_64
    if (*CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_CNTPNSIRQ) {
        // masks the device’s interrupt line
        core_timer_disable();
        // timer IRQ priority = 100
        add_task(core_timer_handler, 100);
        // run tasks
        run_preemptive_tasks();
        // unmasks the device’s interrupt line
        core_timer_enable();
    }
    // from uart (AUX_INT && GPU0) IRQ_EL1h
    else if (*IRQ_PENDING_1 & IRQ_PENDING_1_AUX_INT && *CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_GPU) {
        // https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf (p.13)
        /*
            AUX_MU_IIR_REG on read bits[2:1] :
            00 : No interrupts
            01 : Transmit holding register empty    => can write
            10 : Receiver holds valid byte          => can read
            11: <Not possible> 
        */
        // can write
        if (*AUX_MU_IIR_REG & (0b01 << 1)) {
            // masks the device’s interrupt line (enable by handler)
            disable_mini_uart_w_interrupt();
            // uart IRQ priority = 200
            add_task(uart_interrupt_w_handler, 200);
            // run tasks
            run_preemptive_tasks();
        }
        // can read
        else if (*AUX_MU_IIR_REG & (0b10 << 1)) {
            // masks the device’s interrupt line (enable by handler)
            disable_mini_uart_r_interrupt();
            // uart IRQ priority = 200
            add_task(uart_interrupt_r_handler, 200);
            // run tasks
            run_preemptive_tasks();
        }
        else {
            uart_printf("------ UNKNOWN ------\r\n");
        }
    }
}

void invalid_exception_router(unsigned long long x0) {
    unsigned long long elr_el1;
    asm volatile("mrs %0, ELR_EL1\n\t" : "=r"(elr_el1) :: "memory");
    uart_printf("x0 : %d, ELR_EL1 : %x\r\n", x0, elr_el1);
    while (1);
}

void enable_interrupt() {
    asm volatile("msr daifclr, 0xf");
}

void disable_interrupt() {
    asm volatile("msr daifset, 0xf");
}

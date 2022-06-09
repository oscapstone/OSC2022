#include "exception.h"

void sync64_router(trapframe_t *tpf, unsigned long x1) {
    // for page fault
    esr_el1_t *esr;
    esr = (esr_el1_t *)&x1;
    if (esr->ec == DATA_ABORT_LOWER || esr->ec == INS_ABORT_LOWER) {
        handle_abort(esr);
        return;
    }
    // for sys call
    enable_interrupt();
    unsigned long long syscall_no = tpf->x8;

    if (syscall_no == 0)
        getpid(tpf);
    else if (syscall_no == 1)
        uartread(tpf, (char *)tpf->x0, tpf->x1);
    else if (syscall_no == 2)
        uartwrite(tpf, (char *)tpf->x0, tpf->x1);
    else if (syscall_no == 3)
        exec(tpf, (char *)tpf->x0, (char **)tpf->x1);
    else if (syscall_no == 4)
        fork(tpf);
    else if (syscall_no == 5)
        exit(tpf, tpf->x0);
    else if (syscall_no == 6)
        syscall_mbox_call(tpf, (unsigned char)tpf->x0, (unsigned int *)tpf->x1);
    else if (syscall_no == 7)
        kill(tpf, (int)tpf->x0);
    else if (syscall_no == 8)
        signal_register(tpf->x0, (void (*)())tpf->x1);
    else if (syscall_no == 9)
        signal_kill(tpf->x0, tpf->x1);
    else if (syscall_no == 10)
        sys_mmap(tpf, (void *)tpf->x0, tpf->x1, tpf->x2, tpf->x3, tpf->x4, tpf->x5);
    else if (syscall_no == 31)
        sigreturn(tpf);
}

void irq_router(trapframe_t *tpf) {
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
        
        // at least 2 thread running -> schedule for any timer IRQ
        if (run_queue->next->next != run_queue)
            schedule();
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
            // NOT SURE == clear fifo
            if (!mini_uart_r_interrupt_is_enable()) {
                *AUX_MU_IIR_REG = 0xc2;
                return;
            }
            // masks the device’s interrupt line (enable by handler)
            disable_mini_uart_r_interrupt();
            // uart IRQ priority = 200
            add_task(uart_interrupt_r_handler, 200);
            // run tasks
            run_preemptive_tasks();
        }
        else {
            uart_printf("------ UNKNOWN IRQ from UART ------\r\n");
        }
    }

    // only run signal handler when return to user mode
    if ((tpf->spsr_el1 & 0b1100) == 0) {
        check_signal(tpf);
    }
}

void invalid_exception_router(unsigned long long x0) {
    unsigned long long elr_el1;
    asm volatile("mrs %0, ELR_EL1\n\t" : "=r"(elr_el1) :: "memory");
    uart_printf("x0 : %d, ELR_EL1 : %x\r\n", x0, elr_el1);
    while (1);
}

// 0 => enable, 1 => disable
unsigned long long is_disable_interrupt() {
    unsigned long long daif;
    asm volatile("mrs %0, daif\n\t" : "=r"(daif));
    return daif;
}

static unsigned long long lock_count = 0;
void lock() {
    disable_interrupt();
    lock_count++;
}

void unlock() {
    lock_count--;
    if (lock_count < 0) {
        uart_printf("Error : unlock()\r\n");
        while(1);
    }
    if (lock_count == 0)
        enable_interrupt();
}

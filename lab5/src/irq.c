#include "utils.h"
#include "printf.h"
#include "timer.h"
#include "entry.h"
#include "irq.h"

const char *entry_error_messages[] = {
    "SYNC_INVALID_EL1t",
    "IRQ_INVALID_EL1t",
    "FIQ_INVALID_EL1t",
    "ERROR_INVALID_EL1T",

    "SYNC_INVALID_EL1h",
    "IRQ_INVALID_EL1h",
    "FIQ_INVALID_EL1h",
    "ERROR_INVALID_EL1h",

    "SYNC_INVALID_EL0_64",
    "IRQ_INVALID_EL0_64",
    "FIQ_INVALID_EL0_64",
    "ERROR_INVALID_EL0_64",

    "SYNC_INVALID_EL0_32",
    "IRQ_INVALID_EL0_32",
    "FIQ_INVALID_EL0_32",
    "ERROR_INVALID_EL0_32"};

void show_invalid_entry_message(int type, unsigned long esr, unsigned long address) {
    printf("%s, ESR: %x, address: %x\r\n", entry_error_messages[type], esr, address);
}

void handle_irq(void) {
    unsigned int irq = get32(CORE0_INTERRUPT_SOURCE);
    switch (irq) {
    case (CORE_TIMER_IRQ):
        handle_timer_irq();
        break;
    case (MINI_UART_IRQ):
        // handle_uart_irq();
        break;
    default:
        printf("Unknown pending irq: %x\r\n", irq);
    }
}

#ifndef _IRQ_HANDLER_H_
#define _IRQ_HANDLER_H_

#include "types.h"
#include "peripherals/iomapping.h"
#include "peripherals/mini_uart.h"
#include "lib/print.h"
#include "lib/bitops.h"
#include "kernel/timer.h"
#include "debug/debug.h"
#include "asm.h"

#define MAX_SOFTIRQ_TRY 5 

enum {CORE0_TIMER = 0, MINI_UART_RX, MINI_UART_TX, END_OF_LIST};
typedef void (*irq_funcptr)(void);

struct softirq_status{
    uint16_t pending; // support 16 interrupt
    uint8_t in_softirq;
};
extern uint64_t irq_count[END_OF_LIST];
#endif

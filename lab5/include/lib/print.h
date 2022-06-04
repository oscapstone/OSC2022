#ifndef _PRINT_H_
#define _PRINT_H_

#include "types.h"
#include "kernel/timer.h"
#include "lib/string.h"
#include "peripherals/mini_uart.h"
#include "kernel/irq_handler.h"
#include <stdarg.h>

extern int32_t printf(char *, ...);
extern int32_t putchar(uint8_t);
extern int32_t getchar();

#define INFO(fmt, ...) \
        do{ \
            local_irq_disable(); \
            printf("%l: [%s] " fmt "\r\n" , (int64_t)get_jiffies(), __FUNCTION__, ##__VA_ARGS__); \
            local_irq_enable(); \
        }while(0)
        

#endif

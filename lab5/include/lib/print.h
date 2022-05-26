#ifndef _PRINT_H_
#define _PRINT_H_

#include "types.h"
#include "kernel/timer.h"
#include "lib/string.h"
#include "peripherals/mini_uart.h"
#include <stdarg.h>

int32_t printf(char *, ...);
int32_t putchar(uint8_t);
int32_t getchar();

#define INFO(fmt, ...) \
        //printf("%l: [%s] " fmt "\r\n" , (int64_t)get_jiffies(), __FUNCTION__, ##__VA_ARGS__)

#endif

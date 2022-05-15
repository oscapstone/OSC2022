#ifndef _PRINT_H_
#define _PRINT_H_

#include "types.h"

int32_t printf(char *, ...);
int32_t putchar(uint8_t);
int32_t getchar();

#define INFO(fmt, ...) \
        printf("[%s] " fmt "\r\n" , __FUNCTION__, ##__VA_ARGS__)

#endif

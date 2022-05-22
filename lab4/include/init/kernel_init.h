#ifndef _KERNEL_INIT_H
#define _KERNEL_INIT_H
#include "types.h"
#include "peripherals/mini_uart.h"
#include "fs/initrdfs.h"
#include "kernel/timer.h"
#include "lib/print.h"
#include "lib/simple_malloc.h"
#include "mm/mm.h"

extern void kernel_init(void *);

#endif

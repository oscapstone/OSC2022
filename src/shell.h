#ifndef __SHELL_H__
#define __SHELL_H__

#include "uart.h"
#include "mbox.h"
#include "stddef.h"
#include "cpio.h"
#include "dtb.h"
#include "time.h"
#include "thread.h"
#include "exception.h"

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024
#define CMD_BUF_SIZE 64

char cmd_buf[CMD_BUF_SIZE];

void set(long addr, unsigned int value);
void reset(int tick);
void cancel_reset();
void PrintHelp();
void jump2usr_prog(cpio_fp_t *fp);
void PrintHello();
void PrintInfo();
void clear_screen();
void shell();
void cmd_handler();

#endif

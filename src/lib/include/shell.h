#ifndef __SHELL__H__
#define __SHELL__H__
#include "uart.h"
#include "string.h"
#include "reboot.h"
#include "mbox.h"
#include "cpio.h"
#include "heap.h"
#include "exception.h"
#include "timer.h"
#include "buddy.h"
#include "allocator.h"
#include "schedule.h"
#include "exec.h"

#define MAX_BUFFER_SIZE 256u

void print_sys_info();
void welcome_msg();
void helper();
void cmd_handler(char *cmd);
void cmd_reader(char *cmd);
void exe_shell();

#endif
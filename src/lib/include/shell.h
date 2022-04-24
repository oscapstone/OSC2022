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
#include "list.h"
#include "buddy.h"

#define MAX_BUFFER_SIZE 256u

void print_sys_info();
void welcome_msg();
void helper();
void do_cpiols();
void do_cpiocat();
void cmd_handler(char *cmd);
void cmd_reader(char *cmd);
void exe_shell();
void clear_screen();
void exec (cpio_new_header *header, char *file_name, int enable_timer);

#endif
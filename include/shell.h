#ifndef SHELL_H
#define SHELL_H
#include "cpio.h"
#include "uart.h"
#include "dtb.h"
#include "mbox.h"
#include "reset.h"
#include "string.h"
#include "sched.h"
#include "timer.h"
#include "buddy.h"
#include "malloc.h"

struct cmd {
    char* name;
    void (*func)();
    char* desc;
};

void welcome_msg();
void shell();
void read_cmd();
void exec_cmd();

void cmd_help(char* param);
void cmd_hello(char* param);
void cmd_reboot(char* param);
void cmd_revision();
void cmd_memory();
void cmd_ls(char* param);
void cmd_cat(char* param);
void cmd_dtb(char* param);
void cmd_initramfs();
void cmd_async();
void cmd_prog();
void cmd_sec2();
void cmd_setTimeout(char* param);
void cmd_foo();
void cmd_preempt();
void cmd_pageTest();
void cmd_chunkTest();
void cmd_exec(char* param);
void cmd_unknown();

#endif
#ifndef SHELL_H
#define SHELL_H
#include "cpio.h"
#include "uart.h"
#include "dtb.h"
#include "mbox.h"
#include "reset.h"
#include "string.h"
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
void cmd_preempt();
void cmd_pageTest();
void cmd_chunkTest();
void cmd_unknown();

struct cmd cmd_list[] = {
    {"help",      cmd_help,      "print this help menu"},
    {"hello",     cmd_hello,     "print Hello World!"},
    // {"reboot",    cmd_reboot,    "reboot the device"},
    {"revision",  cmd_revision,  "print board revision"},
    {"memory",    cmd_memory,    "print ARM memory base address and size"},
    {"ls",        cmd_ls,        "list directory contents"},
    {"cat",       cmd_cat,       "print file content"},
    {"dtb",       cmd_dtb,       "show device tree"},
    {"initramfs", cmd_initramfs, "show initramfs address"},
    // {"async",     cmd_async,     "test async print"},
    {"prog",      cmd_prog,      "load a user program in the initramfs, and jump to it"},
    {"sec2",      cmd_sec2,      "print the seconds after booting and set the next timeout to 2 seconds later."},
    {"setTimeout",cmd_setTimeout,"prints message after seconds"},
    // {"preempt",   cmd_preempt,   "test preemption"},
    {"pageTest",  cmd_pageTest,  "test page frame allocator"},
    {"chunkTest", cmd_chunkTest, "test small chunk allocator"},
};

#endif
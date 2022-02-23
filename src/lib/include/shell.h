#ifndef __SHELL__H__
#define __SHELL__H__
#include "reset.h"
#include "utils.h"
#include "uart.h"
#include "string.h"
void print_system_info();
void welcome();
void help();
void clear();
void echo_back(char c);
void read_cmd(char *cmd);
void do_cmd(char *cmd);
void shell_start();
#endif
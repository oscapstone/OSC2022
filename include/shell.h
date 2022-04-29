#include "mini_uart.h"
enum Action{
    help,
    hello,
    revision,
    memory,
    reboot,
    ccreboot,
    bootload,
    version,
    ls,
    cat,
    prog,
    timeout,
    mmalloc,
    mfree,
    mlistc,
    mlistf,
    mtest,
    unknown
};

void read_command();
void read_string(char**);
void handle_command(enum Action action, char *buffer);
int match_command(char *buffer);
int get_para_num(char* s);
char* get_para_by_num(int num,char *buffer);
void read_uart_buf();
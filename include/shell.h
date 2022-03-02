#include "mini_uart.h"
enum Action{
    help,
    hello,
    revision,
    memory,
    reboot,
    ccreboot,
    unknown
};

void read_command();
void handle_command(enum Action action, char *buffer);
int match_command(char *buffer);
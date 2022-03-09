#ifndef CMD_H
#define CMD_H

// for reboot
#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024
void set(long addr, unsigned int value);
void reset(int tick);
void cancel_reset();

void print_welcome();
void cmd_err(char *s);
void cmd_help();
void cmd_hello();
void cmd_revision();
void cmd_memory();
void cmd_reboot();

int readline(char *s, int len);

#endif
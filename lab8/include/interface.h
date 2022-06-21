#ifndef INTERFACE_H
#define INTERFACE_H

#define SYS_getpid 0
#define SYS_uartread 1
#define SYS_uartwrite 2
#define SYS_exec 3
#define SYS_fork 4
#define SYS_exit 5
#define SYS_mbox_call 6
#define SYS_kill 7

#ifndef __ASSEMBLER__

#include <stdint.h>

int getpid();
uint32_t uart_read(char buf[], uint32_t size);
uint32_t uart_write(const char buf[], uint32_t size);
int exec(const char filename[], char *const argv[]);
int fork();
void exit(int status);
int mbox_call(unsigned char ch, uint32_t *mbox);
void kill(int pid);

#endif

#endif

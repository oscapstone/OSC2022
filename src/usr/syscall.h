#ifndef	__SYSCALL__H__
#define	__SYSCALL__H__

#define SYS_GET_PID             0   
#define SYS_UART_READ           1
#define SYS_UART_WRITE          2
#define SYS_EXEC                3
#define SYS_FORK                4
// #define SYS_EXIT                5
// #define SYS_MBOX_CALL           6
// #define SYS_KILL                7

#define NULL (void *)(0)
// #define NULL (char *const)(0)

int getpid();
unsigned long uartread(char buf[], unsigned long size);
unsigned long uartwrite(const char buf[], unsigned long size);
int exec(const char* name, char *const argv[]);
int fork();
// void exit(int status);
// int mbox_call(unsigned char ch, unsigned int *mbox);
// void kill(int pid);

#endif
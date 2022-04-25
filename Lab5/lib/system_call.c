#include "system_call.h"
#include "switch.h"
#include "task.h"
#include "printf.h"

/* helper functions for user programs, not the real system calls */
int get_pid() {
    unsigned long ret;
	asm volatile("svc 0\n");
    asm volatile("mov %0, x0\n":"=r"(ret):);
	return ret;
}

size_t uart_read(char buf[], size_t size) {
    unsigned long ret;
	asm volatile("svc 1\n");
    asm volatile("mov %0, x0\n":"=r"(ret):);
	return ret;
}

size_t uart_write(const char buf[], size_t size) {
    unsigned long ret;
	asm volatile("svc 2\n");
    asm volatile("mov %0, x0\n":"=r"(ret):);
	return ret;
}

int exec(const char *name, char *const argv[]) {
    unsigned long ret;
	asm volatile("svc 3\n");
    asm volatile("mov %0, x0\n":"=r"(ret):);
	return ret;
}

int fork() {
    unsigned long ret;
	asm volatile("svc 4\n");
    asm volatile("mov %0, x0\n":"=r"(ret):);
	return ret;
}

void exit() {
	asm volatile("svc 5\n");
}

int mbox_call(unsigned char ch, unsigned int *mbox) {
    unsigned long ret;
	asm volatile("svc 6\n");
    asm volatile("mov %0, x0\n":"=r"(ret):);
	return ret;
}

void kill(int pid) {
    asm volatile("svc 7\n");
}

unsigned int printf(char* fmt,...) {
	char dst[100];
    __builtin_va_list args;
    __builtin_va_start(args,fmt);
    unsigned int ret = vsprintf(dst,fmt,args);
    uart_write(dst, 100);
    return ret;
}
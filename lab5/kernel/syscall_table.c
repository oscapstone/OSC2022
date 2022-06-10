#include "kernel/syscall_table.h"
void *syscall_table[] = {
    sys_getpid,
    sys_uart_read,
    sys_uart_write,
    0xdeadbeaf,
    sys_fork,
    sys_hello,
    sys_mbox_call,
};


uint64_t len_syscall_table = sizeof(syscall_table) / sizeof(void*);

#include "kernel/syscall_table.h"
void *syscall_table[] = {
    sys_hello,
    sys_uart_read,
    sys_uart_write,
    0,
    sys_fork
};



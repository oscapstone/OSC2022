#ifndef H_DEV_UART
#define H_DEV_UART

#include "vfs.h"

extern file_operations_t dev_uart_file_operations;

int dev_uart_write(file_t *file, const void *buf, size_t len);
int dev_uart_read(file_t *file, void *buf, size_t len);
int dev_uart_open(vnode_t *file_node, file_t **target);
int dev_uart_close(file_t *file);

#endif
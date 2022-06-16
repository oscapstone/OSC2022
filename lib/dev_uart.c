#include "dev_uart.h"
#include "vfs.h"
#include "uart.h"
#include "memory.h"

file_operations_t dev_uart_file_operations = {dev_uart_write, dev_uart_read, dev_uart_open, dev_uart_close, (void *)nop_op, (void *)nop_op};


int dev_uart_write(file_t *file, const void *buf, size_t len) {
    for(int i = 0; i < len; i++) {
        uart_putc(((char*)buf)[i]);
    }
    return len;
}

int dev_uart_read(file_t *file, void *buf, size_t len) {
    for(int i = 0; i < len; i++) {
        do {
            ((char*)buf)[i] = uart_async_getc();
        } while(((char*)buf)[i] == NULL);
    }
    return len;
}

int dev_uart_open(vnode_t *file_node, file_t **target) {
    (*target)->vnode = file_node;
    (*target)->f_ops = &dev_uart_file_operations;
    return 0;
}

int dev_uart_close(file_t *file) {
    free((void*)file);
    return 0;
}
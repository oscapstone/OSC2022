#include <uartfs.h>
#include <uart.h>
#include <read.h>

int uartfs_write(struct file* file, const void* buf, size_t len){
    uart_nbyte((char *)buf, len);
    return len;
}


int uartfs_read(struct file* file, void* buf, size_t len){
    return readnbyte((char *)buf, len);
}
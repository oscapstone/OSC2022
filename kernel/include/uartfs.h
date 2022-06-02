#ifndef UARTFS_H_
#define UARTFS_H_

#include <stddef.h>
#include <vfs.h>
#include <tmpfs.h>

int uartfs_write(struct file* file, const void* buf, size_t len);
int uartfs_read(struct file* file, void* buf, size_t len);



#endif
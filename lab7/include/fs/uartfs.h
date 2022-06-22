#ifndef _UARTFS_H_
#define _UARTFS_H_
#include "fs/vfs.h"
#include "asm.h"
#include "peripherals/mini_uart.h"

extern struct filesystem_type uartfs;
extern struct inode_operations uartfs_i_ops;
extern struct file_operations uartfs_f_ops;
extern struct mount* uartfs_mount(struct filesystem_type*, struct dentry*);
extern struct dentry* uartfs_create(struct dentry *, const char*);
extern struct dentry *uartfs_lookup(struct dentry *, char*);
extern loff_t uartfs_lseek64(struct file *, loff_t, int);
extern long uartfs_read(struct file *, char *, size_t, loff_t *);
extern long uartfs_write(struct file *, char *, size_t, loff_t *);
extern struct file* uartfs_open(struct dentry *,uint32_t , umode_t );
extern int uartfs_flush(struct file *);
extern int uartfs_release(struct inode *, struct file*);
extern int uartfs_mkdir(struct dentry *, const char *, umode_t);

extern struct filesystem_type uartfs;
#endif

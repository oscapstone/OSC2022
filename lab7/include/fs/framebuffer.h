#ifndef _FRAMEBUFFER_H_
#define _FRAMEBUFFER_H_
#include "fs/vfs.h"
#include "asm.h"
#include "peripherals/mailbox.h"

struct framebuffer_info{
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t isrgb;
    uint32_t lfb;
    uint32_t lfb_size;
};

extern struct filesystem_type framebufferfs;
extern struct inode_operations framebufferfs_i_ops;
extern struct file_operations framebufferfs_f_ops;
extern struct mount* framebufferfs_mount(struct filesystem_type*, struct dentry*);
extern struct dentry* framebufferfs_create(struct dentry *, const char*);
extern struct dentry *framebufferfs_lookup(struct dentry *, char*);
extern int framebufferfs_lseek64(struct file *, loff_t, int);
extern long framebufferfs_read(struct file *, char *, size_t, loff_t *);
extern long framebufferfs_write(struct file *, char *, size_t, loff_t *);
extern struct file* framebufferfs_open(struct dentry *,uint32_t , umode_t );
extern int framebufferfs_flush(struct file *);
extern int framebufferfs_release(struct inode *, struct file*);
extern int framebufferfs_mkdir(struct dentry *, const char *, umode_t);
extern int framebufferfs_ioctl(struct file*, unsigned long, va_list);

extern struct filesystem_type framebufferfs;
#endif

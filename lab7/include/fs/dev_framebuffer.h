#ifndef DEV_FRAMEBUFFER_H
#define DEV_FRAMEBUFFER_H

#include "stddef.h"
#include "fs/vfs.h"

int init_dev_framebuffer();

int dev_framebuffer_write(struct file *file, const void *buf, size_t len);
int dev_framebuffer_read(struct file *file, void *buf, size_t len);
int dev_framebuffer_open(struct vnode *file_node, struct file **target);
int dev_framebuffer_close(struct file *file);

#endif
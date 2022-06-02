#ifndef _VFS_H
#define _VFS_H

#include "typedef.h"

#define O_CREAT 00000100

#define REGULAR_FILE 0
#define DIRECTORY 1

#define MAX_PATHNAME_LEN 256
#define MAX_FILESIZE 4096
#define MAX_COMPONENT_NAME_LEN 16 // 15+'\0'
#define MAX_ENTRIES 16
#define MAX_FD_NUM 16

struct mount {
    struct vnode *root;
    struct filesystem *fs;
};

struct vnode {
    struct mount *mount;
    struct vnode_operations *v_ops;
    struct file_operations *f_ops;
    void *internal;
};

// file handle
struct file {
    struct vnode *vnode;
    size_t f_pos; // RW position of this file handle
    struct file_operations *f_ops;
    int flags;
};

struct filesystem {
    const char *name;
    int (*setup_mount)(struct filesystem *fs, struct mount *mount);
};

struct file_operations {
    int (*write)(struct file *file, const void *buf, size_t len);
    int (*read)(struct file *file, void *buf, size_t len);
    int (*open)(struct vnode *file_node, struct file **target);
    int (*close)(struct file *file);
    //long lseek64(struct file *file, long offset, int whence);
};

struct vnode_operations {
    int (*lookup)(struct vnode *dir_node, struct vnode **target,
                  const char *component_name);
    int (*create)(struct vnode *dir_node, struct vnode **target,
                  const char *component_name);
    int (*mkdir)(struct vnode *dir_node, struct vnode **target,
                 const char *component_name);
};

extern struct mount *rootfs;
void rootfs_init(void);
int register_filesystem(struct filesystem *fs);

int vfs_open(const char *pathname, int flags, struct file **target);
int vfs_close(struct file *file);
int vfs_write(struct file *file, const void *buf, size_t len);
int vfs_read(struct file *file, void *buf, size_t len);
int vfs_mkdir(const char *pathname);
int vfs_mount(const char *target, const char *filesystem);
int vfs_lookup(const char *pathname, struct vnode **target);
#endif

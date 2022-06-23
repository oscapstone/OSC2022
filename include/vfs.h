#ifndef VFS_H
#define VFS_H
#include "uart.h"
#include "node.h"
#include "string.h"
#include "malloc.h"
#include "tmpfs.h"

#define MAX_FS_REG 256
#define MAX_PATH_NAME 256
#define O_CREAT 00000100

struct vnode {
    struct mount *mount;
    struct vnode_operations *v_ops;
    struct file_operations *f_ops;
    void *internal;
};

// file handle
struct file {
    struct vnode *vnode;
    unsigned long f_pos; // RW position of this file handle
    struct file_operations *f_ops;
    int flags;
};

// mounted file system
struct mount {
    struct vnode *root;
    struct filesystem *fs;
};

struct filesystem {
    const char *name;
    int (*setup_mount)(struct filesystem *fs, struct mount *mount);
};

struct file_operations {
    int (*write)(struct file *file, const void *buf, unsigned long len);
    int (*read)(struct file *file, void *buf, unsigned long len);
    int (*open)(struct vnode *file_node, struct file **target);
    int (*close)(struct file *file);
    long (*getsize)(struct vnode *vnode);
};

struct vnode_operations {
    int (*lookup)(struct vnode *dir_node, struct vnode **target,
                  const char *component_name);
    int (*create)(struct vnode *dir_node, struct vnode **target,
                  const char *component_name);
    int (*mkdir)(struct vnode *dir_node, struct vnode **target,
                 const char *component_name);
};

struct mount *rootfs;
struct filesystem reg_fs[MAX_FS_REG];

int register_filesystem(struct filesystem *fs);
struct filesystem *find_filesystem(const char *fs_name);

// file operations
int vfs_write(struct file *file, const void *buf, unsigned long len);
int vfs_read(struct file *file, void *buf, unsigned long len);
int vfs_open(const char *pathname, int flags, struct file **target);
int vfs_close(struct file *file);
// vnode operations
int vfs_lookup(const char *pathname, struct vnode **target);
int vfs_mkdir(const char *pathname);
int vfs_mount(const char *target, const char *filesystem);

void init_rootfs();
char* path_to_absolute(char* path, char* curr_working_dir);

#endif
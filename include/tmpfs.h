#ifndef TMPFS_H
#define TMPFS_H
#include "vfs.h"
#include "node.h"
#include "uart.h"
#include "malloc.h"

#define FILE_NAME_MAX 32
#define MAX_DIR_ENTRY 32
#define MAX_FILE_SIZE 4096

struct tmpfs_inode {
    enum node_type type;
    char name[FILE_NAME_MAX];
    struct vnode *entry[MAX_DIR_ENTRY];
    char *data;
    unsigned long datasize;
};

int register_tmpfs();
int tmpfs_setup_mount(struct filesystem *fs, struct mount *_mount);
struct vnode *tmpfs_create_vnode(struct mount *_mount, enum node_type type);

int tmpfs_write(struct file *file, const void *buf, unsigned long len);
int tmpfs_read(struct file *file, void *buf, unsigned long len);
int tmpfs_open(struct vnode *file_node, struct file **target);
int tmpfs_close(struct file *file);
long tmpfs_getsize(struct vnode *vnode);

int tmpfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name);
int tmpfs_create(struct vnode *dir_node, struct vnode **target, const char *component_name);
int tmpfs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name);

#endif
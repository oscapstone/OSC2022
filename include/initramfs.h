#ifndef H_INITRAMFS
#define H_INITRAMFS

#include "vfs.h"

typedef struct initramfs_internal {
    enum NodeType ntype;
    char name[MAX_COMPONENT_LENGTH];
    vnode_t *entry[MAX_ENTRY];
    char *data;
    uint64 datasize;
} initramfs_internal_t;

filesystem_t *get_initramfs();
int setup_initramfs_mount(filesystem_t *fs, mount_t *mount);
vnode_t *create_initramfs_vnode(mount_t *mount, enum NodeType ntype);
int initramfs_write(file_t *file, const void *buf, size_t len);
int initramfs_read(file_t *file, void *buf, size_t len);
int initramfs_open(vnode_t *file_node, file_t **target);
int initramfs_close(file_t *file);
int initramfs_getsize(vnode_t *target);

int initramfs_lookup(vnode_t *dir, vnode_t **target, const char *component_name);
int initramfs_create(vnode_t *dir, vnode_t **target, const char *component_name);
int initramfs_mkdir(vnode_t *dir, vnode_t **target, const char *component_name);






#endif
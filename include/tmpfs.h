#ifndef H_TMPFS
#define H_TMPFS

#include "vfs.h"

typedef struct tmpfs_internal {
    enum NodeType ntype;
    char name[MAX_COMPONENT_LENGTH];
    vnode_t *entry[MAX_ENTRY];
    char *data;
    uint64 datasize;
} tmpfs_internal_t;

filesystem_t *get_tmpfs();
int setup_tmpfs_mount(filesystem_t *fs, mount_t *mount);
vnode_t *create_tmpfs_vnode(mount_t *mount, enum NodeType ntype);
int tmpfs_write(file_t *file, const void *buf, size_t len);
int tmpfs_read(file_t *file, void *buf, size_t len);
int tmpfs_open(vnode_t *file_node, file_t **target);
int tmpfs_close(file_t *file);
int tmpfs_getsize(vnode_t *target);

int tmpfs_lookup(vnode_t *dir, vnode_t **target, const char *component_name);
int tmpfs_create(vnode_t *dir, vnode_t **target, const char *component_name);
int tmpfs_mkdir(vnode_t *dir, vnode_t **target, const char *component_name);


#endif
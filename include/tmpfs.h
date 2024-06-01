#ifndef _TMPFS_HEADER_
#define _TMPFS_HEADER_
#include "vfs.h"
#include "utils.h"

#define COMP_NAME_LEN 16
#define FILE_BLOCK_SIZE 4096
#define MAX_ENTRIES 16


enum tmpfs_node_type{
    dir_n,
    file_n,
    mount_fs,
};
struct tmpfs_inode{
    enum tmpfs_node_type type;
    struct tmpfs_block* data;
    char name[COMP_NAME_LEN];
    size_t size;
    struct vnode* vnode;
    struct tmpfs_inode* parent;
    struct tmpfs_inode* next_sibling;
    struct tmpfs_inode* child;
};

struct tmpfs_block{
    char* content;
    struct tmpfs_block* next;
};

int tmpfs_register();
int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount);
int tmpfs_read(struct file* file, void* buf, size_t len);
int tmpfs_write(struct file* file, const void* buf, size_t len);
int tmpfs_lookup(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
int tmpfs_create(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
int tmpfs_close(struct file* file);
int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target,
              const char* component_name);
void tmpfs_ls(struct vnode* dir_node);
char* get_pos_block_addr(int pos,struct tmpfs_inode* inode);
#endif
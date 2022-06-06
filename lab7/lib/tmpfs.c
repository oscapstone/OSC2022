#include "tmpfs.h"
#include "mm.h"
#include "string.h"
#include "mini_uart.h"


struct vnode_operations *tmpfs_v_ops;
struct file_operations *tmpfs_f_ops;

int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount) {

    struct vnode *root_node = (struct vnode *)chunk_alloc(sizeof(struct vnode));
    struct tmpfs_internal *root_internal = (struct tmpfs_internal *)chunk_alloc(sizeof(struct tmpfs_internal));

    root_internal->parent = root_internal;
    root_internal->size = 0;
    root_internal->type = DIRECTORY;
    root_internal->vnode = root_node;
    root_internal->data = 0;

    root_node->f_ops = tmpfs_f_ops;
    root_node->v_ops = tmpfs_v_ops;
    root_node->internal = (void *)root_internal;
    root_node->mount = mount;

    mount->root = root_node;    
    mount->fs = fs;

    printf("[debug] setup root: 0x%x\n", mount);
    printf("[debug] setup root vnode: 0x%x\n", root_node);

    return 0;
}

int tmpfs_register(struct mount *mnt, struct vnode *root) {

    root->v_ops = (struct vnode_operations *) chunk_alloc(sizeof(struct vnode_operations));
    root->f_ops = (struct file_operations *) chunk_alloc(sizeof(struct file_operations));

    root->v_ops->lookup = tmpfs_lookup;
    root->v_ops->create = tmpfs_create;
    root->v_ops->mkdir = tmpfs_mkdir;

    root->f_ops->open = tmpfs_open;
    root->f_ops->read = tmpfs_read;
    root->f_ops->write = tmpfs_write;
    root->f_ops->close = tmpfs_close;

    return 0;
}

int tmpfs_open(struct vnode* file_node, struct file** target) {
    return SUCCESS;
}

int tmpfs_close(struct file *file) {
    if (file)
        return SUCCESS;
    else 
        return FAIL;
}

int tmpfs_write(struct file *file, const void *buf, unsigned len) {
    struct tmpfs_internal *internal = (struct tmpfs_internal*)file->vnode->internal;
    if (((struct tmpfs_internal *)file->vnode->internal)->type != REGULAR_FILE)
        return FAIL;

    char *dest = &((char *)internal->data)[file->f_pos];
    char *src = (char *)buf;
    int i = 0;
    for (; i < len && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = EOF;

    return i;
}

int tmpfs_read(struct file *file, void *buf, unsigned len) {
    struct tmpfs_internal *internal = (struct tmpfs_internal*)file->vnode->internal;
    if (internal->type != REGULAR_FILE)
        return FAIL;
    
    char *dest = (char*)buf;
    char *src = &((char *)internal->data)[file->f_pos];
    int i = 0;
    for (; i<len && &src[i] < ((char *)internal->data)+MAX_FILESIZE; i++) {
        dest[i] = src[i];
    }

    return i;
}

int tmpfs_create(struct vnode* dir_node, struct vnode** target, const char* component_name) {
    struct tmpfs_internal *file_node = (struct tmpfs_internal *)chunk_alloc(sizeof(struct tmpfs_internal));
    struct vnode *new_node = (struct vnode *)chunk_alloc(sizeof(struct vnode));

    strcpy(file_node->name, component_name);
    file_node->type = REGULAR_FILE;
    file_node->parent = (struct tmpfs_internal *)dir_node->internal;
    file_node->vnode = new_node;
    file_node->size = 0;
    file_node->data = malloc(MAX_FILESIZE);

    file_node->vnode->f_ops = tmpfs_f_ops;
    file_node->vnode->internal = (void *)file_node;
    file_node->vnode->mount = 0;
    file_node->vnode->v_ops = 0;

    struct tmpfs_internal *parent_internal = (struct tmpfs_internal *)dir_node->internal;
    parent_internal->child[parent_internal->size] = file_node;
    parent_internal->size++;

    *target = file_node->vnode;
    return SUCCESS;
}

int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name) {
    struct tmpfs_internal *dir_internal = (struct tmpfs_internal *)chunk_alloc(sizeof(struct tmpfs_internal));
    struct  vnode *new_node = (struct vnode *)chunk_alloc(sizeof(struct vnode));

    strcpy(dir_internal->name, component_name);
    dir_internal->type = DIRECTORY;
    dir_internal->parent = (struct tmpfs_internal *)dir_node->internal;
    dir_internal->vnode = new_node;
    dir_internal->size = 0;
    dir_internal->data = 0;

    dir_internal->vnode->f_ops = tmpfs_f_ops;
    dir_internal->vnode->internal = (void *)dir_internal;
    dir_internal->vnode->mount = 0;
    dir_internal->vnode->v_ops = tmpfs_v_ops;

    struct tmpfs_internal *parent_internal = (struct tmpfs_internal *)dir_node->internal;
    parent_internal->child[parent_internal->size] = dir_internal;
    parent_internal->size++;

    *target = dir_internal->vnode;
    return SUCCESS;
}

int tmpfs_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name) {
    struct tmpfs_internal *internal = (struct tmpfs_internal *)dir_node->internal;
    if (internal->type != DIRECTORY) {
        return FAIL;
    }
    
    for (int i=0; i<internal->size; i++) {
        if(stringcmp(internal->child[i]->name, component_name) == 0) {
            *target = internal->child[i]->vnode;
            return i;
        }
    }

    return FAIL;
}

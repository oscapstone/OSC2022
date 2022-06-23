#include "tmpfs.h"

struct file_operations tmpfs_file_operations = {tmpfs_write, tmpfs_read, tmpfs_open, tmpfs_close, tmpfs_getsize};
struct vnode_operations tmpfs_vnode_operations = {tmpfs_lookup, tmpfs_create, tmpfs_mkdir};

int register_tmpfs() {
    struct filesystem fs;
    fs.name = "tmpfs";
    fs.setup_mount = tmpfs_setup_mount;
    return register_filesystem(&fs);
}

int tmpfs_setup_mount(struct filesystem *fs, struct mount *_mount) {
    _mount->fs = fs;
    _mount->root = tmpfs_create_vnode(0, dir_t);
    return 0;
}

struct vnode* tmpfs_create_vnode(struct mount* _mount, enum node_type type) {
    struct vnode *v = kmalloc(sizeof(struct vnode));
    v->f_ops = &tmpfs_file_operations;
    v->v_ops = &tmpfs_vnode_operations;
    v->mount = 0;
    struct tmpfs_inode* inode = kmalloc(sizeof(struct tmpfs_inode));
    memset(inode, 0, sizeof(struct tmpfs_inode));
    inode->type = type;
    inode->data = kmalloc(0x1000);
    v->internal = inode;
    return v;
}

int tmpfs_write(struct file *file, const void *buf, unsigned long len) {
    // copy data into inode & update f_pos
    struct tmpfs_inode *inode = file->vnode->internal;
    memcpy(inode->data + file->f_pos, buf, len);
    file->f_pos += len;
    // update datasize
    if (inode->datasize < file->f_pos)
        inode->datasize = file->f_pos;
    return len;
}

int tmpfs_read(struct file *file, void *buf, unsigned long len) {
    struct tmpfs_inode *inode = file->vnode->internal;
    // read until EOF
    if (file->f_pos + len > inode->datasize) {
        len = inode->datasize - file->f_pos;
    }
    memcpy(buf, inode->data + file->f_pos, len);
    file->f_pos += len;
    return len;
}

int tmpfs_open(struct vnode *file_node, struct file **target) {
    (*target)->vnode = file_node;
    (*target)->f_ops = file_node->f_ops;
    (*target)->f_pos = 0;
    return 0;
}

int tmpfs_close(struct file *file) {
    kfree(file);
    return 0;
}

int tmpfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name) {
    struct tmpfs_inode *dir_inode = dir_node->internal;
    int child_idx = 0;
    // iterate child entry
    for (; child_idx < MAX_DIR_ENTRY; child_idx++) {
        struct vnode *vnode = dir_inode->entry[child_idx];
        // if there is no such child
        if (!vnode)
            continue;
        // lookup success
        struct tmpfs_inode *inode = vnode->internal;
        if (strcmp(component_name, inode->name) == 0) {
            *target = vnode;
            return 0;
        }
    }
    // uart_printf("[WARNING] tmpfs_lookup not found in this level\r\n");
    return -1;
}

int tmpfs_create(struct vnode *dir_node, struct vnode **target, const char *component_name) {
    // check vnode type
    struct tmpfs_inode *inode = dir_node->internal;
    if (inode->type != dir_t) {
        uart_printf("[ERROR] tmpfs_create FAIL, dir_node is not dir_t\r\n");
        return -1;
    }
    // iterate child
    int child_idx = 0;
    for (; child_idx < MAX_DIR_ENTRY; child_idx++) {
        // if there is an empty entry
        if (!inode->entry[child_idx])
            break;
        // prevent identical file name
        struct tmpfs_inode *child_inode = inode->entry[child_idx]->internal;
        if (strcmp(child_inode->name, component_name) == 0) {
            uart_printf("[ERROR] tmpfs_create FAIL, file already exist\r\n");
            return -1;
        }
    }
    // assign new vnode to child_idx
    struct vnode *tmp_vnode = tmpfs_create_vnode(0, file_t);
    inode->entry[child_idx] = tmp_vnode;
    // assign name
    struct tmpfs_inode *tmp_inode = tmp_vnode->internal;
    memcpy(tmp_inode->name, component_name, strlen(component_name));
    *target = tmp_vnode;
    return 0;
}

int tmpfs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name) {
    // check vnode type
    struct tmpfs_inode *inode = dir_node->internal;
    if (inode->type != dir_t) {
        uart_printf("[ERROR] tmpfs_mkdir FAIL, dir_node is not dir_t\r\n");
        return -1;
    }
    // find first empty child entry
    int child_idx = 0;
    for (; child_idx < MAX_DIR_ENTRY; child_idx++) {
        if (!inode->entry[child_idx])
            break;
    }
    // assign new vnode to child_idx
    struct vnode* tmp_vnode = tmpfs_create_vnode(0, dir_t);
    inode->entry[child_idx] = tmp_vnode;
    // assign name
    struct tmpfs_inode *tmp_inode = tmp_vnode->internal;
    memcpy(tmp_inode->name, component_name, strlen(component_name));
    *target = tmp_vnode;
    return 0;
}

long tmpfs_getsize(struct vnode* vnode) {
    struct tmpfs_inode *inode = vnode->internal;
    return inode->datasize;
}

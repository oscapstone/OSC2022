#include "vfs.h"
#include "tmpfs.h"
#include "string.h"
#include "mm.h"
#include "mini_uart.h"

struct mount *rootfs;

void rootfs_init() {
    
    struct filesystem *tmpfs = (struct filesystem *)chunk_alloc(sizeof(struct filesystem));
    tmpfs->name = (char *)chunk_alloc(TMPFS_COMP_LEN);
    strcpy(tmpfs->name, "tmpfs");
    tmpfs->setup_mount = tmpfs_setup_mount;
    register_fs(tmpfs);
    
    rootfs = (struct mount *)chunk_alloc(sizeof(struct mount));    
    tmpfs->setup_mount(tmpfs, rootfs);

    printf("[debug] confirm root vnode: 0x%x\n", rootfs->root);

}

int register_fs(struct filesystem *fs) {
    if (stringcmp(fs->name, "tmpfs")) {
        return tmpfs_register();
    }
    return -1;
}

int vfs_open(const char *pathname, int flags, struct file **target) {
    printf("[debug] start of vfs_open\n");
    *target = 0;
    struct vnode *target_dir;
    char target_path[VFS_PATHMAX];
    traverse(pathname, &target_dir, target_path);
    printf("[debug] traverse complete\n");
    struct vnode *target_file;
    if (target_dir->v_ops->lookup(target_dir, &target_file, target_path) == 0) {
        printf("[debug] vfs_open: open\n");
        *target = (struct file *)chunk_alloc(sizeof(struct file));
        (*target)->vnode = target_file;
        (*target)->f_pos = 0;
        (*target)->f_ops = target_file->f_ops;
        (*target)->flags = flags;
        return (*target)->f_ops->open(target_file, target);
    } else if (flags & O_CREAT) {
        printf("[debug] vfs_open: create and open\n");
        int res = target_dir->v_ops->create(target_dir, &target_file, target_path);
        if (res < 0) return FAIL;
        *target = (struct file *)chunk_alloc(sizeof(struct file));
        (*target)->vnode = target_file;
        (*target)->f_pos = 0;
        (*target)->f_ops = target_file->f_ops;
        (*target)->flags = flags;
        return (*target)->f_ops->open(target_file, target);
    } else return FAIL;
}

int vfs_close(struct file *file) {
    int code = file->f_ops->close(file);
    if (code == SUCCESS)
        chunk_free(file);
    return code;
}

int vfs_write(struct file *file, const void *buf, unsigned len) {
    return file->f_ops->write(file, buf, len);
}

int vfs_read(struct file *file, void *buf, unsigned len) {
    return file->f_ops->read(file, buf, len);
}

int vfs_mkdir(const char *pathname) {
    struct vnode *target_dir;
    char child_name[VFS_PATHMAX];
    traverse(pathname, &target_dir, child_name);
    struct vnode *child_dir;
    int res = target_dir->v_ops->mkdir(target_dir, &child_dir, child_name);
    if (res < 0) return res;
    return SUCCESS;
}

int vfs_mount(const char *target, const char *filesystem) {
    return SUCCESS;
}

int vfs_lookup(const char *pathname, struct vnode **target) {
    return SUCCESS; // unused
}

int vfs_chdir(const char *pathname) {
    struct vnode *target_dir;
    char path_remain[VFS_PATHMAX];
    traverse(pathname, &target_dir, path_remain);
    if (stringcmp(path_remain, "") == 0) {
        return FAIL;
    } else {
        current->cwd = target_dir;
        return SUCCESS;
    }
}

void traverse(const char* pathname, struct vnode **target_node, char *target_path) {
    if (pathname[0] == '/') {
        printf("[debug] traverse absolute path: %s\n", pathname);
        printf("[debug] root: 0x%x\n", rootfs);
        printf("[debug] root vnode: 0x%x\n", rootfs->root);
        struct vnode *rootnode = rootfs->root;
        printf("[debug] here -1\n");
        r_traverse(rootnode, pathname + 1, target_node, target_path);
    } else {
        printf("[debug] traverse relative path\n");
    }
}

void r_traverse(struct vnode *node, const char *path, struct vnode **target_node, char *target_path) {
    printf("[debug] begin r_traverse\n");
    int i = 0;
    while (path[i]) {
        if (path[i] == '/') break;
        target_path[i] = path[i];
        i++;
    }
    printf("[debug] here\n");
    target_path[i++] = '\0';
    *target_node = node;

    printf("[debug] r_traverse target path: %s\n", target_path);

    if (stringcmp(target_path, "") == 0) {
        return;
    }
    else if (stringcmp(target_path, ".") == 0) {

    }
    else if (stringcmp(target_path, "..") == 0) {

    }

    int res = node->v_ops->lookup(node, target_node, target_path);
    if (res != FAIL)
        r_traverse(*target_node, path+i, target_node, target_path);
    
}

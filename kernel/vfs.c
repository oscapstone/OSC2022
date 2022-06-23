#include "vfs.h"

int register_filesystem(struct filesystem *fs) {
    // find first empty reg_fs entry
    for (int i = 0; i < MAX_FS_REG; i++) {
        if(!reg_fs[i].name) {
            reg_fs[i].name = fs->name;
            reg_fs[i].setup_mount = fs->setup_mount;
            return i;
        }
    }
    // reg_fs is full
    return -1;
}

struct filesystem* find_filesystem(const char* fs_name) {
    // find file system by name
    for (int i = 0; i < MAX_FS_REG; i++) {
        if (strcmp(reg_fs[i].name, fs_name) == 0)
            return &reg_fs[i];
    }
    return 0;
}

int vfs_write(struct file *file, const void *buf, unsigned long len) {
    // 1. write len byte from buf to the opened file.
    // 2. return written size or error code if an error occurs.
    return file->f_ops->write(file, buf, len);
}

int vfs_read(struct file *file, void *buf, unsigned long len) {
    // 1. read min(len, readable size) byte to buf from the opened file.
    // 2. block if nothing to read for FIFO type
    // 3. return read size or error code if an error occurs.
    return file->f_ops->read(file, buf, len);
}

int vfs_open(const char *pathname, int flags, struct file **target) {
    // 1. Lookup pathname
    // 3. Create a new file if O_CREAT is specified in flags and vnode not found
    struct vnode *node;
    if (vfs_lookup(pathname, &node) != 0 && (flags & O_CREAT)) {
        // find last slash in pathname
        int last_slash_idx = 0;
        for (int i = 0; i < strlen(pathname); i++) {
            if(pathname[i] == '/')
                last_slash_idx = i;
        }
        // find corresponding dirname from pathname
        char dirname[MAX_PATH_NAME];
        memcpy(dirname, pathname, strlen(pathname));
        dirname[last_slash_idx] = 0;
        // lookup dirname
        if (vfs_lookup(dirname, &node) != 0) {
            uart_printf("[ERROR] O_CREAT FAIL, no such directory name\r\n");
            return -1;
        }
        // there is dirname => O_CREAT
        node->v_ops->create(node, &node, pathname + last_slash_idx + 1);
        // create a new file handle for this vnode
        *target = kmalloc(sizeof(struct file));
        node->f_ops->open(node, target);
        (*target)->flags = flags;
        return 0;
    }
    // 2. Create a new file handle for this vnode if found.
    else {
        *target = kmalloc(sizeof(struct file));
        node->f_ops->open(node, target);
        (*target)->flags = flags;
        return 0;
    }
    // lookup error code shows if file exist or not or other error occurs
    // 4. Return error code if fails
    return -1;
}

int vfs_close(struct file *file) {
    // 1. release the file handle
    // 2. Return error code if fails
    file->f_ops->close(file);
    return 0;
}

int vfs_lookup(const char *pathname, struct vnode **target) {
    // root node
    if (strlen(pathname) == 0) {
        *target = rootfs->root;
        return 0;
    }
    // search from root node
    struct vnode *dirnode = rootfs->root;
    char component_name[FILE_NAME_MAX+1];
    memset(component_name, 0, FILE_NAME_MAX+1);
    int c_idx = 0;
    // iterate directory
    for (int i = 1; i < strlen(pathname); i++) {
        // delimiter
        if (pathname[i] == '/') {
            // end of component name
            component_name[c_idx++] = 0;
            // if there is no such dirname, return -1
            if (dirnode->v_ops->lookup(dirnode, &dirnode, component_name) != 0)
                return -1;
            // redirect to new mounted file system
            while (dirnode->mount) {
                dirnode = dirnode->mount->root;
            }
            // reset index
            c_idx = 0;
        }
        // copy pathname
        else {
            component_name[c_idx++] = pathname[i];
        }
    }
    // last component
    component_name[c_idx++] = 0;
    if (dirnode->v_ops->lookup(dirnode, &dirnode, component_name) != 0)
        return -1;
    // redirect to new mounted file system
    while (dirnode->mount) {
        dirnode = dirnode->mount->root;
    }

    *target = dirnode;
    return 0;
}

int vfs_mkdir(const char *pathname) {
    char dirname[MAX_PATH_NAME], newdirname[MAX_PATH_NAME];
    memset(dirname, 0, MAX_PATH_NAME);
    memset(newdirname, 0, MAX_PATH_NAME);
    // find last slash in pathname
    int last_slash_idx = 0;
    for (int i = 0; i < strlen(pathname); i++) {
        if (pathname[i] == '/')
            last_slash_idx = i;
    }
    // divide into 2 parts
    memcpy(dirname, pathname, last_slash_idx);
    memcpy(newdirname, pathname + last_slash_idx + 1, strlen(pathname + last_slash_idx + 1));

    struct vnode *node;
    // if there is dirname => mkdir
    if (vfs_lookup(dirname, &node) == 0) {
        node->v_ops->mkdir(node, &node, newdirname);
        return 0;
    }
    // if there is no such dirname
    uart_printf("[ERROR] vfs_mkdir FAIL, no such directory name\r\n");
    return -1;
}

int vfs_mount(const char *target, const char *filesystem) {
    struct vnode *dirnode;
    struct filesystem *fs = find_filesystem(filesystem);
    // if there is no such file system
    if (!fs) {
        uart_printf("[ERROR] vfs_mount FAIL, no such file system\r\n");
        return -1;
    }
    // if there is no such dirname
    if (vfs_lookup(target, &dirnode) == -1) {
        uart_printf("[ERROR] vfs_mount FAIL, no such directory name\r\n");
        return -1;
    }
    // mount
    else {
        dirnode->mount = kmalloc(sizeof(struct mount));
        fs->setup_mount(fs, dirnode->mount);
    }
    return 0;
}

void init_rootfs() {
    // register tmpfs as rootfs
    int idx = register_tmpfs();
    rootfs = kmalloc(sizeof(struct mount));
    reg_fs[idx].setup_mount(&reg_fs[idx], rootfs);
}

char* path_to_absolute(char* path, char* curr_working_dir) {
    // uart_printf("\r\n[TEST in path2abs] path = %s, curr = %s\r\n\r\n", path, curr_working_dir);
    // if path is relative path
    if (path[0] != '/') {
        char tmp[MAX_PATH_NAME];
        memset(tmp, 0, MAX_PATH_NAME);
        memcpy(tmp, curr_working_dir, strlen(curr_working_dir));
        if (strcmp(curr_working_dir, "/") != 0) {
            tmp[strlen(tmp)] = '/';
        }
        // concatenate curr_working_dir & path
        memcpy(tmp + strlen(tmp), path, strlen(path));
        memset(path, 0, strlen(path));
        memcpy(path, tmp, strlen(tmp));
        path[strlen(path)] = 0;
    }

    char absolute_path[MAX_PATH_NAME];
    memset(absolute_path, 0, MAX_PATH_NAME);
    int idx = 0;
    for (int i = 0; i < strlen(path); i++) {
        // if there is /..
        if (path[i] == '/' && path[i+1] == '.' && path[i+2] == '.') {
            // find previous /
            for (int j = idx; j >= 0; j--) {
                if(absolute_path[j] == '/') {
                    absolute_path[j] = 0;
                    idx = j;
                }
            }
            i += 2;
            continue;
        }
        // ignore /.
        if (path[i] == '/' && path[i+1] == '.') {
            i++;
            continue;
        }
        // normal case
        absolute_path[idx++] = path[i];
    }
    // end of absolute_path
    absolute_path[idx] = 0;

    memset(path, 0, strlen(path));
    memcpy(path, absolute_path, strlen(absolute_path));
    return path;
}

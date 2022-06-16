#include "vfs.h"
#include "tmpfs.h"
#include "initramfs.h"
#include "dev_uart.h"
#include "dev_framebuffer.h"
#include "memory.h"
#include "string.h"

static uint32 num_registered = 0;
int register_filesystem(filesystem_t *fs) {
    // register the file system to the kernel.
    // you can also initialize memory pool of the file system here.

    // already registered
    for(int i = 0; i < num_registered; i++) {
        if(strcmp(fs->name, registered_fs[i].name)) {
            return i;
        }
    }

    // new register
    if(num_registered < MAX_REG_FS) {
        registered_fs[num_registered].name = fs->name;
        registered_fs[num_registered].setup_mount = fs->setup_mount;
        return num_registered++;
    }
    // too many registered fs 
    else {
        return -1;
    }
}

filesystem_t *get_filesystem(const char *name) {
    for(int i = 0; i < num_registered; i++) {
        if(strcmp(name, registered_fs[i].name)) {
            return &registered_fs[i];
        }
    }

    return NULL;
}

int vfs_open(const char* pathname, int flags, struct file** target) {
    // 1. Lookup pathname
    // 2. Create a new file handle for this vnode if found.
    // 3. Create a new file if O_CREAT is specified in flags and vnode not found
    // lookup error code shows if file exist or not or other error occurs
    // 4. Return error code if fails

    // uart_printf("(vfs) open %s - start\n", pathname);
    vnode_t *searchNode;
    bool isExisted = vfs_lookup(pathname, &searchNode) == 0;
    if(!isExisted) {
        if(flags & O_CREAT) {
            // uart_printf("(vfs) O_CREAT %s\n", pathname);

            char dirname[MAX_PATHNAME];
            char *basename = get_dirname(dirname, pathname);
            if(vfs_lookup(dirname, &searchNode) != 0) {
                uart_printf("[ERROR] (vfs) fail to ocreate, dir doesn't exist\n");
                return -1;
            }

            if(searchNode->v_ops->create(searchNode, &searchNode, basename) == -1) {
                uart_printf("[ERROR] (vfs) fail to create %s\n", pathname);
                return -1;
            }

            // uart_printf("(vfs) create %s\n", pathname);
        }
        else {
            uart_printf("[ERROR] (vfs) fail to open, pathname doesn't exist\n");
            return -1;
        }
    }

    *target = (file_t*)malloc(sizeof(file_t));
    if(searchNode->f_ops->open(searchNode, target) == -1) {
        uart_printf("(vfs) open fail: %s\n", pathname);
        return -1;
    }
    (*target)->flags = flags;
    // uart_printf("(vfs) open %s - success \n", pathname);
    return 0;
}

int vfs_close(struct file* file) {
    // 1. release the file handle
    // 2. Return error code if fails
    // uart_printf("(vfs) close 0x%x\n", file);
    return file->f_ops->close(file);
}

int vfs_write(struct file* file, const void* buf, size_t len) {
    // 1. write len byte from buf to the opened file.
    // 2. return written size or error code if an error occurs.
    // uart_printf("(vfs) write\n");
    return file->f_ops->write(file, buf, len);
}

int vfs_read(struct file* file, void* buf, size_t len) {
    // 1. read min(len, readable size) byte to buf from the opened file.
    // 2. block if nothing to read for FIFO type
    // 2. return read size or error code if an error occurs.

    // uart_printf("vfs read(%d): ", len);
    return file->f_ops->read(file, buf, len);
}

long vfs_lseek64(struct file* file, long offset, int whence) {
    if(whence == SEEK_SET) {
        file->f_pos = offset;
        return file->f_pos;
    }
    return -1;
}


int vfs_lookup(const char *pathname, vnode_t **target) {
    char rootname[MAX_PATHNAME];
    const char *leaftname = pathname;
    
    if(strlen(pathname) == 0) {
        // uart_printf("(vfs) lookup root\n");
        *target = rootfs->root;
        return 0;
    }

    vnode_t *dir = rootfs->root;
    while(1) {   
        leaftname = get_rootname(rootname, leaftname);
        if(dir->v_ops->lookup(dir, &dir, rootname) == -1) {
            uart_printf("[ERROR] (vfs) fail to lookup: %s (%s, %s)\n", pathname, rootname, leaftname);
            return -1;
        }

        while(dir->mount) dir = dir->mount->root; // if mount then ignore all other entries
        
        if(leaftname == NULL) {
            *target = dir;
            // uart_printf("(vfs) lookup %s\n", pathname);
            return 0;
        }
    }
}

int vfs_mkdir(const char *pathname) {
    char dirname[MAX_PATHNAME];
    char *basename = get_dirname(dirname, pathname);
    // uart_printf("(vfs) mkdir %s\n", pathname);

    vnode_t *searchNode;
    if(vfs_lookup(dirname, &searchNode) == -1) {
        uart_printf("[ERROR] (vfs) fail to mkdir, parenet dir doesn't exist\n");
        return -1;
    }

    searchNode->v_ops->mkdir(searchNode, &searchNode, basename);
    return 0;
}

int vfs_mount(const char *target, const char *filesystem) {
    filesystem_t *fs = get_filesystem(filesystem);

    // uart_printf("(vfs) mount %s at %s - start\n", filesystem, target);

    if(!fs) {
        uart_printf("[ERROR] (vfs) %s is not registered\n", filesystem);
        return -1;
    }

    vnode_t *searchNode;
    if(vfs_lookup(target, &searchNode) == -1) {
        uart_printf("[ERROR] (vfs) %s doesn't exist\n", target);
        return -1;
    }

    // uart_printf("(vfs) mount %s at %s - success\n", filesystem, target);
    searchNode->mount = (mount_t*)malloc(sizeof(mount_t));
    fs->setup_mount(fs, searchNode->mount);
    return 0;
}

int nop_op() {
    return -1;
}


static uint32 num_registered_fop = 0;
int register_dev_fop(file_operations_t *fop) {
    if(num_registered_fop < MAX_REG_DEV) {
        registered_dev_fops[num_registered_fop] = *fop;
        return num_registered_fop++;
    }
    // too many registered fop 
    else {
        return -1;
    }
}

int vfs_mknod(const char *pathname, int dev_id) {
    file_t *dev_file;
    if(vfs_open(pathname, O_CREAT, &dev_file) == -1) {
        uart_printf("[ERROR] mknod fail on %s with id: %d\n", pathname, dev_id);
        return -1;
    }
    
    dev_file->vnode->f_ops = &registered_dev_fops[dev_id];
    free((void*)dev_file);
    return 0;
}

void init_vfs() {
    rootfs = malloc(sizeof(mount_t));
    filesystem_t *tmpfs = get_tmpfs();
    int id = register_filesystem(tmpfs);
    registered_fs[id].setup_mount(&registered_fs[id], rootfs);
    free((void*)tmpfs);

    vfs_mkdir("/initramfs");
    filesystem_t *initramfs = get_initramfs();
    id = register_filesystem(initramfs);
    vfs_mount("/initramfs", "initramfs");
    free((void*)initramfs);

    vfs_mkdir("/dev");
    int dev_uart_id = register_dev_fop(&dev_uart_file_operations);
    vfs_mknod("/dev/uart", dev_uart_id);

    init_dev_framebuffer();
    int dev_framebuffer_id = register_dev_fop(&dev_framebuffer_file_operations);
    vfs_mknod("/dev/framebuffer", dev_framebuffer_id);
}
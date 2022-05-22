#include <vfs.h>
#include <malloc.h>
#include <string.h>
#include <tmpfs.h>
#include <uart.h>

Mount *rootfs;

void rootfs_init(char *fs_name){
    if(strcmp(fs_name, "tmpfs") == 0){
        FileSystem *tmpfs = (FileSystem *)kmalloc(sizeof(FileSystem));
        tmpfs->name = (char *)kmalloc(sizeof(char) * 6); // 6 is the length of "tmpfs"
        strcpy(tmpfs->name, "tmpfs");
        tmpfs->setup_mount = tmpfs_setup_mount;
        int err = register_filesystem(tmpfs);
        if(err){
            uart_puts("Failed to register tmpfs filesystem\n");
        }
    }
}


int register_filesystem(FileSystem *fs) {
    // register the file system to the kernel.
    // you can also initialize memory pool of the file system here.
    if(fs == NULL) return -1;
    if(fs->name == NULL || fs->setup_mount == NULL) return -1;
    if(strcmp(fs->name, "tmpfs") == 0){
        tmpfs_set_ops();
    }
    fs->setup_mount(fs, rootfs);

    return 0;
}

int vfs_open(const char* pathname, int flags, struct file** target) {
    // 1. Lookup pathname
    // 2. Create a new file handle for this vnode if found.
    // 3. Create a new file if O_CREAT is specified in flags and vnode not found
    // lookup error code shows if file exist or not or other error occurs
    // 4. Return error code if fails
    return 0;
}

int vfs_close(struct file* file) {
    // 1. release the file handle
    // 2. Return error code if fails
    return 0;

}

int vfs_write(struct file* file, const void* buf, size_t len) {
    // 1. write len byte from buf to the opened file.
    // 2. return written size or error code if an error occurs.
    return 0;
}

int vfs_read(struct file* file, void* buf, size_t len) {
    // 1. read min(len, readable size) byte to buf from the opened file.
    // 2. block if nothing to read for FIFO type
    // 2. return read size or error code if an error occurs.
    return 0;
}


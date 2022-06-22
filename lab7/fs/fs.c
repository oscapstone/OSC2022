#include "fs/fs.h"

void fs_init(){
    vfs_init(&tmpfs);
    register_filesystem(&tmpfs);
    register_filesystem(&uartfs);

    vfs_mkdir("/dev", 0);
    vfs_mkdir("/dev/uart", 0);
    vfs_mount(vfs_lookup("/dev/uart"), "uartfs"); 
}

// syscall number : 11
int sys_open(const char *pathname, int flags){
    FS_LOG("sys_open");
    struct task_struct* current = get_current();
    struct files_struct *files = current->files;
    struct file* f;
    int fd, ret;
    
    fd = get_unused_fd(files); 
    if(fd == -1 || ((f = vfs_open(pathname, flags, 0)) == NULL)){
        put_unused_fd(files, fd); 
        ret = -1;
    }else{
        fd_install(files, fd, f);
        ret = fd;
    }
    FS_LOG("return fd: %d", ret);
    return ret;
}

// syscall number : 11
int sys_close(int fd){
    FS_LOG("sys_close(%p)", fd);
    if(fd > NR_OPEN_DEFAULT || fd < 0) return -1; 
    struct task_struct* current = get_current();
    struct files_struct* files = current->files;
    struct file* file = get_file_by_fd(files, fd); 
    int ret = -1;
    
    if(file != NULL){
        ret = vfs_close(file);
        if(ret != -1){
            put_unused_fd(files, fd); 
        }
    }
    return ret;
}

// syscall number : 13
// remember to return read size or error code
long sys_write(int fd, char *buf, unsigned long count){
    FS_LOG("sys_write(%p, %p, %p)",fd, buf, count);
    if(fd > NR_OPEN_DEFAULT || fd < 0) return -1; 
    struct task_struct* current = get_current();
    struct file* file = get_file_by_fd(current->files, fd);

    return vfs_write(file, buf, count);
}

// syscall number : 14
// remember to return read size or error code
long sys_read(int fd, char *buf, unsigned long count){
    FS_LOG("sys_read(%p, %p, %p)",fd, buf, count);
    if(fd > NR_OPEN_DEFAULT || fd < 0) return -1; 
    struct task_struct* current = get_current();
    struct file* file = get_file_by_fd(current->files, fd);

    return vfs_read(file, buf, count);
}

// syscall number : 15
// you can ignore mode, since there is no access control
int sys_mkdir(const char *pathname, unsigned mode){
    FS_LOG("sys_mkdir");

    return vfs_mkdir(pathname, mode);
}

// syscall number : 16
// you can ignore arguments other than target (where to mount) and filesystem (fs name)
int sys_mount(const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data){
    FS_LOG("sys_mount");
    int ret = -1;
    struct dentry* target_dir;
    target_dir = vfs_lookup(target);
    if(target_dir != NULL && S_ISDIR(target_dir->d_inode->i_modes)){
        ret = vfs_mount(target_dir, filesystem);  
    }
    return ret;
}

// syscall number : 17
int sys_chdir(const char *path){
    FS_LOG("sys_chdir");
    struct dentry* dest_dir;
    struct task_struct* current = get_current();
    dest_dir = vfs_lookup(path);
    
    if(dest_dir == NULL || !S_ISDIR(dest_dir->d_inode->i_modes)) {
        return -1;
    }else{
        current->fs->pwd = dest_dir;
        FS_LOG("PWD: %s", current->fs->pwd->d_name);
        return 0;
    }
}

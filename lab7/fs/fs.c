#include "fs/fs.h"

void fs_init(){
    vfs_init(&tmpfs);
    register_filesystem(&tmpfs);
}

// syscall number : 11
int sys_open(const char *pathname, int flags){
    INFO("sys_open");
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
    INFO("return fd: %d", ret);
}

// syscall number : 11
int sys_close(int fd){
    INFO("sys_close");
}

// syscall number : 13
// remember to return read size or error code
long sys_write(int fd, const void *buf, unsigned long count){
    INFO("sys_write");
}

// syscall number : 14
// remember to return read size or error code
long sys_read(int fd, void *buf, unsigned long count){
    INFO("sys_read");
}

// syscall number : 15
// you can ignore mode, since there is no access control
int sys_mkdir(const char *pathname, unsigned mode){
    INFO("sys_mkdir");
}

// syscall number : 16
// you can ignore arguments other than target (where to mount) and filesystem (fs name)
int sys_mount(const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data){
    INFO("sys_mount");
}

// syscall number : 17
int sys_chdir(const char *path){
    INFO("sys_chdir");
}

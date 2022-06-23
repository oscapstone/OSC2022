#include "fs/uartfs.h"

struct filesystem_type uartfs = {
    .fs_name = "uartfs",
    .mount = uartfs_mount
};

struct inode_operations uartfs_i_ops = {
	.create = uartfs_create,
    .lookup = uartfs_lookup,
    .mkdir = uartfs_mkdir
};

struct file_operations uartfs_f_ops = {
    .lseek64 = uartfs_lseek64,
    .read =  uartfs_read,
    .write = uartfs_write,
    .open = uartfs_open,
    .flush = uartfs_flush,
    .release = uartfs_release
};

struct dentry* uartfs_create(struct dentry * parent, const char* new_file_name){
    FS_LOG("uartfs_create");
    return NULL;
}

struct dentry *uartfs_lookup(struct dentry *parent, char* target){
    FS_LOG("uartfs_lookup");
    return NULL;
}

int uartfs_lseek64(struct file *, loff_t, int){
    FS_LOG("uartfs_lseek64");
}

long uartfs_read(struct file *file, char *buf, size_t len, loff_t *offset){
    return uart_read(buf, len);
}

long uartfs_write(struct file * file, char * buf, size_t len, loff_t * offset){
    return uart_write(buf, len);
}

struct file* uartfs_open(struct dentry* d_file,uint32_t flags,umode_t mode){
    FS_LOG("uartfs_open");
    return create_file(d_file, flags, mode); 
}
int uartfs_flush(struct file *){
    FS_LOG("uartfs_flush");
    return 0;
}
int uartfs_release(struct inode * inode, struct file * file){
    FS_LOG("uartfs_release");
    if(file == NULL) return -1;

    file->f_count--;
    if(file->f_count == 0) kfree(file); 
    return 0;
}
struct mount* uartfs_mount(struct filesystem_type* fs_type, struct dentry* target){
    FS_LOG("uartfs_mount");
    uint64_t daif = local_irq_disable_save();
    struct dentry* new_root, *link; 
    new_root->d_parent = target->d_parent;
    struct inode* root_node = create_inode(&uartfs_f_ops, &uartfs_i_ops, S_IFCHR);
    struct mount* ret = (struct mount*)kmalloc(sizeof(struct mount));

    new_root = create_dentry("uartfs", 0, root_node);
    target->d_mnt = ret;
    ret->mnt_root = new_root;

    local_irq_restore(daif);
    return ret;
}


int uartfs_mkdir(struct dentry * parent, const char * target, umode_t mode){
    return -1;
}

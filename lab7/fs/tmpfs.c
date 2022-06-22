#include "fs/tmpfs.h"

struct filesystem_type tmpfs = {
    .f_name = "tmpfs",
    .mount = tmpfs_mount
};

struct inode_operations tmpfs_i_ops = {
	.create = tmpfs_create,
    .lookup = tmpfs_lookup
};

struct file_operations tmpfs_f_ops = {
    .lseek64 = tmpfs_lseek64,
    .read =  tmpfs_read,
    .write = tmpfs_write,
    .open = tmpfs_open,
    .flush = tmpfs_flush,
    .release = tmpfs_release
};

int tmpfs_create(struct dentry * parent, struct dentry * new_file){
    INFO("tmpfs_create");
    struct dentry* ent;
    uint64_t daif = local_irq_disable_save();
    struct inode* parent_inode = parent->d_inode;
    struct tmpfs_dir* parent_dir = parent_inode->private_data;
    struct inode* new_inode = new_file->d_inode;
    for(uint64_t i = 0 ; i < MAX_NUM_DIR_ENTRY ; i++){
        if(parent_dir->entries[i] == NULL){
            new_inode->private_data = kmalloc(sizeof(struct tmpfs_file));
            memset(new_inode->private_data, 0, sizeof(struct tmpfs_dir));
            parent_dir->entries[i] = new_file; 
            local_irq_restore(daif);
            INFO("tmpfs_create: %p", new_file);
            return 0;
        }
    } 
    local_irq_restore(daif);
    return -1;
}
struct dentry *tmpfs_lookup(struct dentry *parent, char* target){
    INFO("tmpfs_lookup");
    struct dentry* ent;
    uint64_t daif = local_irq_disable_save();
    struct inode* parent_inode = parent->d_inode;
    struct tmpfs_dir* parent_dir = parent_inode->private_data;

    for(uint64_t i = 0 ; i < MAX_NUM_DIR_ENTRY ; i++){
        ent = parent_dir->entries[i]; 
        if(ent != NULL && strcmp(ent->d_name, target) == 0){
            local_irq_restore(daif);
            return ent;
        }
    } 
    local_irq_restore(daif);
    return NULL;
}

loff_t tmpfs_lseek64(struct file *, loff_t, int){
    INFO("tmpfs_lseek64");
}

ssize_t tmpfs_read(struct file *, char *, size_t, loff_t *){
    INFO("tmpfs_read");
}
ssize_t tmpfs_write(struct file *, char *, size_t, loff_t *){
    INFO("tmpfs_write");
}
int tmpfs_open(struct inode *, struct file *){
    INFO("tmpfs_open");
}
int tmpfs_flush(struct file *){
    INFO("tmpfs_flush");
}
int tmpfs_release(struct inode *, struct file *){
    INFO("tmpfs_release");
}
struct mount* tmpfs_mount(struct filesystem_type* fs_type, struct dentry* mnt_root){
    INFO("tmpfs_mount");
    uint64_t daif = local_irq_disable_save();
    struct inode* root_node = mnt_root->d_inode;
    struct mount* ret = (struct mount*)kmalloc(sizeof(struct mount));
    ret->mnt_root = mnt_root;
    root_node->f_ops = &tmpfs_f_ops; 
    root_node->i_ops = &tmpfs_i_ops; 
    local_irq_restore(daif);
    return ret;
}

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

struct dentry* create_tmpfs_file(const char* name){
    char *new_file_name = kmalloc(strlen(name));
    struct dentry* ret;
    struct tmpfs_file * tmp_file;
    strcpy(new_file_name, name);

    // create dentry
    ret = create_dentry(new_file_name, 0, NULL);

    // create and initialzie inode
    ret->d_inode = create_inode(&tmpfs_f_ops, &tmpfs_i_ops, S_IFREG);
    ret->d_inode->private_data = kmalloc(sizeof(struct tmpfs_file));
    
    // create file data space
    tmp_file = ret->d_inode->private_data;
    tmp_file->size = 0;
    tmp_file->data = calloc_page();

    return ret;
}

struct tmpfs_dir* create_tmpfs_dir(){
    struct tmpfs_dir* ret = kmalloc(sizeof(struct tmpfs_dir));
    memset(ret, 0, sizeof(struct tmpfs_dir));
    return ret;
}

struct dentry* tmpfs_create(struct dentry * parent, const char* new_file_name){
    FS_LOG("tmpfs_create");
    struct dentry* ent;
    uint64_t daif = local_irq_disable_save();
    struct inode* parent_inode = parent->d_inode;
    struct tmpfs_dir* parent_dir = parent_inode->private_data;
    struct dentry* new_file;

    for(uint64_t i = 0 ; i < MAX_NUM_DIR_ENTRY ; i++){
        if(parent_dir->entries[i] == NULL){
            new_file = create_tmpfs_file(new_file_name);
            parent_dir->entries[i] = new_file;
            local_irq_restore(daif);
            return new_file;
        }
    } 
    local_irq_restore(daif);
    return NULL;
}

struct dentry *tmpfs_lookup(struct dentry *parent, char* target){
    FS_LOG("tmpfs_lookup");
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
    FS_LOG("tmpfs_lseek64");
}

long tmpfs_read(struct file *file, char *buf, size_t len, loff_t *offset){
    FS_LOG("tmpfs_read");
    uint64_t daif = local_irq_disable_save();
    struct dentry* d_file = file->f_dentry;
    struct tmpfs_file* tmp_file = d_file->d_inode->private_data;
    char* data = tmp_file->data;
    size_t file_size = tmp_file->size;
    if(*offset + len > file_size) len = file_size - *offset;

    memcpy(buf, &data[*offset], len);
    *offset += len;

    local_irq_restore(daif);
    return len;
}

long tmpfs_write(struct file * file, char * buf, size_t len, loff_t * offset){
    FS_LOG("tmpfs_write");
    uint64_t daif = local_irq_disable_save();
    struct dentry* d_file = file->f_dentry;
    struct tmpfs_file* tmp_file = d_file->d_inode->private_data;
    char* data = tmp_file->data;
    if(*offset + len > MAX_FILE_SIZE) len = MAX_FILE_SIZE - *offset;

    memcpy(&data[*offset], buf, len);
    *offset += len;
    tmp_file->size = *offset;

    local_irq_restore(daif);
    return len;
}

int tmpfs_open(struct inode *, struct file *){
    FS_LOG("tmpfs_open");
}
int tmpfs_flush(struct file *){
    FS_LOG("tmpfs_flush");
    return 0;
}
int tmpfs_release(struct inode * inode, struct file * file){
    FS_LOG("tmpfs_release");
    if(file == NULL) return -1;

    file->f_count--;
    if(file->f_count == 0) kfree(file); 
}
struct mount* tmpfs_mount(struct filesystem_type* fs_type, struct dentry* mnt_root){
    FS_LOG("tmpfs_mount");
    uint64_t daif = local_irq_disable_save();
    struct inode* root_node = mnt_root->d_inode;
    struct mount* ret = (struct mount*)kmalloc(sizeof(struct mount));
    ret->mnt_root = mnt_root;
    root_node->f_ops = &tmpfs_f_ops; 
    root_node->i_ops = &tmpfs_i_ops; 
    local_irq_restore(daif);
    return ret;
}

#include "mm/mm.h"
#include "fs/vfs.h"

struct mount* rootfs;
LIST_HEAD(mount_list);
LIST_HEAD(filesystem_type_list);

int get_unused_fd(struct files_struct* files){
    int fd = -1;
    uint64_t daif = local_irq_disable_save();
    for(int i = 0 ; i < NR_OPEN_DEFAULT ; i++){
        if(files->fd_array[i] == NULL){
            fd = i;
            files->fd_array[i] = (struct file*)1;
            break;
        }
    }
    local_irq_restore(daif);
    return fd;
}

int put_unused_fd(struct files_struct* files, int fd){
    uint64_t daif = local_irq_disable_save();
    files->fd_array[fd] = NULL;
    local_irq_restore(daif);
}

int fd_install(struct files_struct* files, int fd, struct file *f){
    uint64_t daif = local_irq_disable_save();
    files->fd_array[fd] = f;
    local_irq_restore(daif);
}

void vfs_init(struct filesystem_type* fs_type){
    struct inode* inode_root;
    struct dentry* root_dir;
    struct mount* root_mnt;
    char* name = kmalloc(4);
    strcpy(name, "/");
    // create root filesystem
    inode_root = create_inode(NULL, NULL, S_IFDIR);
    root_dir = create_dentry(name, 0, inode_root);

    root_mnt = fs_type->mount(fs_type, root_dir); 
    
    // add root mount to mount list
    list_add_tail(&root_mnt->list, &mount_list);

    // set rootfs
    rootfs = root_mnt;
}

struct inode* create_inode(struct file_operations *f_ops, struct inode_operations *i_ops, umode_t i_modes){
    struct inode* ret = (struct inode*)kmalloc(sizeof(struct inode));
    ret->i_modes = i_modes;
    ret->i_ops = i_ops;
    ret->f_ops = f_ops;
    ret->private_data = NULL;
    return ret;
}

struct dentry* create_dentry(char *d_name, int d_flags, struct inode* d_inode){
    struct dentry* ret = (struct dentry*)kmalloc(sizeof(struct dentry));
    ret->d_name = d_name;
    ret->d_flags = d_flags;

    ret->d_inode = d_inode;
    return ret;
}

struct file* create_file(struct dentry* d, unsigned int flags, unsigned mode){
    struct file* ret = (struct file*)kmalloc(sizeof(struct file));
    ret->f_dentry = d;
    ret->f_count = 1;
    ret->f_flags = flags;
    ret->f_ops = d->d_inode->f_ops;
    ret->f_mode = mode;

    return ret;
}

struct file* vfs_open(const char* pathname, int flags, umode_t mode){
    const char* delim = "/";
    char* name, *prev_tok;
    struct task_struct* current = get_current();
    struct dentry* prev = NULL, *next, *pwd, *new_dentry;
    struct inode* new_inode;
    char* tok, *new_file_name;
    
    if(pathname[0] == '\0') goto error; 
    
    if(pathname[0] == '/'){
        next = rootfs->mnt_root; 
        pathname++;
    }else{
        current = get_current();
        pwd = current->fs->pwd;
        next = pwd;
    }
    
    name = (char*)kmalloc(strlen(pathname));
    strcpy(name, pathname);

    tok = strtok(name, delim);
    while (tok != NULL) {
        INFO(" %s\n", tok);
        if(next == NULL || !S_ISDIR(next->d_flags)) goto free;
        prev_tok = tok;
        prev = next;
        next = next->d_inode->i_ops->lookup(next, tok);  
        tok = strtok(NULL, delim);
    }
    
    if(next == NULL && (flags | O_CREAT)){
        new_file_name = kmalloc(strlen(prev_tok));
        strcpy(new_file_name, prev_tok);
        new_dentry = create_dentry(new_file_name, S_IFREG, NULL);
        new_inode = create_inode(NULL, NULL, 0);
        if(prev->d_inode->i_ops->create(new_inode, new_dentry, mode) != -1) next = new_dentry;
        else goto free;
    }

    if(next != NULL && S_ISREG(next->d_flags)) goto found;

free:
    kfree(name);
error:
    return NULL;
found:
    kfree(name);
    return create_file(next, flags, mode);
}

int vfs_close(struct file* file){

}

int vfs_write(struct file* file, const void* buf, ssize_t len){
    
}

int vfs_read(struct file* file, void* buf, ssize_t len){

}

int vfs_lookup(const char* pathname, struct dentry* dir){

}

int vfs_mount(const char* target, const char* filesystem){

}

int register_filesystem(struct filesystem_type* fs){
    // register the file system to the kernel.
    // you can also initialize memory pool of the file system here.
    list_add_tail(&fs->list, &filesystem_type_list);
}

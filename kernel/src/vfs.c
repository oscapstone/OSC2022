#include <vfs.h>
#include <malloc.h>
#include <string.h>
#include <tmpfs.h>
#include <uart.h>

char *global_dir;
Dentry *global_dentry;
Mount *rootfs;
FileSystem *fs_pool[MAX_FS_NUM];

void rootfs_init(char *fs_name){
    for(unsigned int idx = 0; idx < MAX_FS_NUM; idx++){
        FileSystem *init_fs = (FileSystem *)kmalloc(sizeof(FileSystem));
        init_fs->name = NULL;
        init_fs->setup_mount = NULL;
        fs_pool[idx] = init_fs;
    }

    if(strcmp(fs_name, "tmpfs") == 0){
        fs_pool[0]->name = (char *)kmalloc(sizeof(char) * 6); // 6 is the length of "tmpfs"  
        strcpy(fs_pool[0]->name, "tmpfs");
        fs_pool[0]->setup_mount = tmpfs_setup_mount;
    }

    int err = register_filesystem(fs_pool[0]);
    if(err) uart_puts("[x] Failed to register filesystem\n");
    
    rootfs = (Mount *)kmalloc(sizeof(Mount));
    fs_pool[0]->setup_mount(fs_pool[0], rootfs);

    global_dir = (char *)kmalloc(sizeof(char) * 2046);
    strcpy(global_dir, "/");
    global_dentry = rootfs->root_dentry;
}


int register_filesystem(FileSystem *fs) {
    // register the file system to the kernel.
    // you can also initialize memory pool of the file system here.
    if(fs == NULL) return -1;
    if(fs->name == NULL || fs->setup_mount == NULL) return -1;
    if(strcmp(fs->name, "tmpfs") == 0){
        tmpfs_set_ops();
        uart_puts("[*] Registered tmpfs\n");
    }
    else{
        tmpfs_set_ops();
        uart_puts("[*] Registered another fs\n");
    }

    return 0;
}

void find_component_name(const char *pathname, char *component_name, char delimiter){
    int i = 0;
    while(pathname[i] != delimiter && pathname[i] != '\0'){
        component_name[i] = pathname[i];
        i++;
    }
    component_name[i] = '\0';
}

int vfs_lookup(const char* pathname, Dentry **target_path, VNode **target_vnode, char *component_name) {
    int ready_return = 0;
    int idx;
    // 1. Lookup pathname
    if(pathname[0] == '/'){
        idx = 1;
        *target_path = rootfs->root_dentry;
    }
    else{
        idx = 0;
        *target_path = global_dentry;
    }

    char tmp_buf[MAX_PATHNAME_LEN];
    find_component_name(pathname + idx, component_name, '/');
    while(component_name[0] != '\0'){
        /* 
        * ready_return is 1 beacuse it couldn't find child vnode before
        * The next component name is not '\0' now, it's not the last component name
        * Therefore, it is a wrong pathname, even if the flag is O_CREAT, it also cannot create a new file
        */
        if(ready_return) return -1;
        
        /* Try to find the child vnode */
        int err = rootfs->root_dentry->vnode->v_ops->lookup((*target_path)->vnode, target_vnode, component_name);
        if(err){
            ready_return = 1;
        } 
        /* save the component name in tmp_buf beacuse it will check the last name */
        strcpy(tmp_buf, component_name);

        /* find next component name*/
        idx += strlen(component_name) + 1;
        find_component_name(pathname + idx, component_name, '/');

        /* find the next vnode */
        if(*target_vnode != NULL){
            /* check the dir is mountpoint or not */
            if((*target_vnode)->dentry->type == D_MOUNT){
                *target_path = (*target_vnode)->dentry->mount->root_dentry;
            }
            else{
                /* the next vnode is child vnode*/
                *target_path = (*target_vnode)->dentry;
            }
            
        } 
    }
    /* if the last component name is '\0', it is a file name */
    strcpy(component_name, tmp_buf);

    return 0;
}

int vfs_open(const char* pathname, int flags, struct file** target_file) {
    if(pathname == NULL) return -1;
    Dentry *target_path = NULL;
    VNode *target_vnode = NULL;
    char component_name[MAX_PATHNAME_LEN];
    int err = vfs_lookup(pathname, &target_path, &target_vnode, component_name);
    if(err) return -1; // worng pathname
    // 2. Create a new file handle for this vnode if found.
    if(target_vnode != NULL){
        if(target_vnode->dentry->type == D_DIR || 
            target_vnode->dentry->type == D_MOUNT) return -2; // cannot open a directory
        err = rootfs->root_dentry->vnode->f_ops->open(target_vnode, target_file);
        if(err == -1) return err;
        return 0;
    } 
    // // 3. Create a new file if O_CREAT is specified in flags and vnode not found
    // // lookup error code shows if file exist or not or other error occurs
    else{
        if(flags & O_CREAT){
            err = rootfs->root_dentry->vnode->v_ops->create(target_path->vnode, &target_vnode, component_name);
            if(err) return err;
            err = rootfs->root_dentry->vnode->f_ops->open(target_vnode, target_file);
            if(err) return err;
            uart_puts("[*] Created file: ");
            uart_puts((*target_file)->vnode->dentry->name);
            uart_puts("\n");
            return 0;
        }
    }

    // 4. Return error code if fails
    return -1;
}


int vfs_mkdir(const char *pathname){
    if(pathname == NULL) return -1;
    Dentry *target_path = NULL;
    VNode *target_vnode = NULL;
    char component_name[MAX_PATHNAME_LEN];
    int err = vfs_lookup(pathname, &target_path, &target_vnode, component_name);
    if(err) return -1; // worng pathname

    if(target_vnode != NULL) return -2; // folder already exist

    err = rootfs->root_dentry->vnode->v_ops->mkdir(target_path->vnode, &target_vnode, component_name);
    if(err) return -1;

    return 0;
}

int vfs_ls(const char *pathname){
    /* now path */
    Dentry *target_path = NULL;
    VNode *target_vnode = NULL;
    char component_name[MAX_PATHNAME_LEN];

    if(pathname == NULL){
        target_path = global_dentry;
        target_vnode = global_dentry->vnode;
    }
    else{
        int err = vfs_lookup(pathname, &target_path, &target_vnode, component_name);
        if(err) return -1; // worng pathname
    }

    /* file/folder not exist */
    if(target_vnode == NULL) return -2; 

    /* print the file/folder name */
    return rootfs->root_dentry->vnode->v_ops->ls(target_vnode->dentry);
}

int vfs_chdir(const char *pathname){
    Dentry *target_path = NULL;
    VNode *target_vnode = NULL;
    char component_name[MAX_PATHNAME_LEN];
    if(pathname == NULL){
        global_dentry = rootfs->root_dentry;
        strcpy(global_dir, "/");
        target_path = global_dentry;
        target_vnode = global_dentry->vnode;
        return 0;
    }
    else{
        int err = vfs_lookup(pathname, &target_path, &target_vnode, component_name);
        if(err) return -1; // worng pathname
        if(target_vnode->dentry->type == D_FILE) return -2; // cannot change to a file
    }

    /* file/folder not exist */
    if(target_vnode == NULL) return -2;

    /* change the global_dentry and global_dir */
    return rootfs->root_dentry->vnode->v_ops->chdir(target_vnode->dentry);
}

int vfs_mount(const char *pathname, const char *filesystem){
    if(filesystem == NULL) return -1;
    Dentry *target_path = NULL;
    VNode *target_vnode = NULL;
    char component_name[MAX_PATHNAME_LEN];
    int err;
    if(pathname == NULL){
        target_path = global_dentry;
        target_vnode = global_dentry->vnode;
    }
    else{
        err = vfs_lookup(pathname, &target_path, &target_vnode, component_name);
        if(err) return -1; // worng pathname
    }
    
    /* target vnode isn't exist, cannot mount it */
    if(target_vnode == NULL) return -2;
        
    /* target vnode is exist, can mount it */
    FileSystem *target_fs = NULL;
    unsigned int idx = 0;
    /* find the filesystem */
    for(; idx < MAX_FS_NUM; idx++){
        if(fs_pool[idx] == NULL) break;
        if(strcmp(filesystem, fs_pool[idx]->name) == 0){
            target_fs = fs_pool[idx];
            goto MOUNT_FS;
        }
    }
    /* cannot find the filesystem , use the empty fs and register it*/
    fs_pool[idx]->name = (char *)kmalloc(sizeof(char) * 6); // 6 is the length of "tmpfs"  
    strcpy(fs_pool[idx]->name, filesystem);
    fs_pool[idx]->setup_mount = tmpfs_setup_mount;
    target_fs = fs_pool[idx];
    err = register_filesystem(target_fs);
    if(err) uart_puts("[x] Failed to register another filesystem\n");

MOUNT_FS:;
    /* mount the filesystem */
    Mount *new_mount = (Mount *)kmalloc(sizeof(Mount));
    target_fs->setup_mount(target_fs, new_mount);
    new_mount->mount_parent = target_path->mount;
    target_vnode->dentry->type = D_MOUNT;

    return 0;
}

int vfs_close(struct file* file) {
    // 1. release the file handle
    // 2. Return error code if fails
    if(file == NULL) return -1;
    return file->f_ops->close(file);
}

int vfs_write(struct file* file, const void* buf, size_t len) {
    // 1. write len byte from buf to the opened file.
    // 2. return written size or error code if an error occurs.
    if(file == NULL) return -1;
    return file->f_ops->write(file, buf, len);
}

int vfs_read(struct file* file, void* buf, size_t len) {
    // 1. read min(len, readable size) byte to buf from the opened file.
    // 2. block if nothing to read for FIFO type
    // 2. return read size or error code if an error occurs.
    if(file == NULL) return -1;
    return file->f_ops->read(file, buf, len);
}



// int traversal_path(const char *pathname, Dentry *target_path, char *target_name){
//     find_target_name(pathname, target_name, '/');
//     if(*target_name == '\0'){
//         /* if the target name is empty, it is end of the directory */
//         return 0;
//     } 
//     else if(strcmp(target_name, ".")){
//         /* if the target name is ".", it is the same directory */
//         return traversal_path(pathname + strlen(target_name) + 1, target_path, target_name);
//     }
//     else if(strcmp(target_name, "..")){
//         /* if dentry is root path, return it */
//         if(target_path->parent == NULL) return 0;
//         /* if dentry is not root path, find its parent */
//         return traversal_path(pathname + strlen(target_name) + 1, target_path->parent, target_name);
//     }
//     else{
//         /* need to find the child dentry */
//         struct list_head *pos;
//         list_for_each(pos, &target_path->childs){
//             Dentry *tmp = (Dentry *)pos;
//             if(strcmp(tmp->name, target_name) == 0){
//                 // TODO: need to check the dir is other filesystem

//                 if(target_path->type == D_DIR){
//                     return traversal_path(pathname + strlen(target_name) + 1, tmp, target_name);
//                 }

//                 /* if the target is a file, break and return it */
//                 return 0; 
//             }
//         }
//     }
//     /* if the target is not found, return error code */
//     return -1;
// } 


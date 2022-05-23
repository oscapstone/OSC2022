#include <tmpfs.h>
#include <malloc.h>
#include <string.h>

struct file_operations* tmpfs_file_ops;
struct vnode_operations* tmpfs_vnode_ops;

int tmpfs_setup_mount(FileSystem *fs, Mount *mount){
    mount->fs = fs;
    mount->root_dentry = tmpfs_create_dentry("/", NULL, D_DIR); 
    return 0;
}

Dentry *tmpfs_create_dentry(const char *name, Dentry *parent, enum dentry_type type){
    Dentry *new_dentry = (Dentry *)kmalloc(sizeof(Dentry));
    new_dentry->name = (char *)kmalloc(sizeof(char) * strlen(name));
    strcpy(new_dentry->name, name);
    new_dentry->parent = parent;
    if(new_dentry->parent != NULL){
        /* add to parent's child list */
        list_add_tail(&parent->childs, &new_dentry->list); 
    }
    new_dentry->type = type;
    new_dentry->mount = NULL;
    new_dentry->vnode = tmpfs_create_vnode(new_dentry);
    return new_dentry;
}

VNode *tmpfs_create_vnode(Dentry *dentry){
    VNode *new_vnode = (VNode *)kmalloc(sizeof(VNode));
    new_vnode->dentry = dentry;
    new_vnode->v_ops = tmpfs_vnode_ops;
    new_vnode->f_ops = tmpfs_file_ops;
    new_vnode->internal = NULL;
    return new_vnode;
}

void tmpfs_set_ops(){
    tmpfs_file_ops = (struct file_operations *)kmalloc(sizeof(struct file_operations));
    tmpfs_vnode_ops = (struct vnode_operations *)kmalloc(sizeof(struct vnode_operations));

    tmpfs_file_ops->write = tmpfs_write;
    tmpfs_file_ops->read = tmpfs_read;
    tmpfs_file_ops->open = tmpfs_open;
    tmpfs_file_ops->close = tmpfs_close;
    tmpfs_file_ops->lseek64 = tmpfs_lseek64;

    tmpfs_vnode_ops->lookup = tmpfs_lookup;
    tmpfs_vnode_ops->create = tmpfs_create;
    tmpfs_vnode_ops->mkdir = tmpfs_mkdir;
}

int tmpfs_write(struct file* file, const void* buf, size_t len){

    return 0;
}
int tmpfs_read(struct file* file, void* buf, size_t len){

    return 0;
}
int tmpfs_open(struct vnode* file_node, struct file** target){

    return 0;
}
int tmpfs_close(struct file* file){

    return 0;
}
long tmpfs_lseek64(struct file* file, long offset, int whence){

    return 0;
}


int tmpfs_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name){
    // if(*component_name == '\0'){
    //     /* if the target name is empty, it is end of the directory */
    //     return 0;
    // } 
    if(strcmp(component_name, ".")){
        /* if the target name is ".", it is the same directory */
        *target = dir_node;
        return 0;
    }
    else if(strcmp(component_name, "..")){
        /* if dentry is root path, return it */
        if(dir_node->dentry->parent == NULL) return 0;
        /* if dentry is not root path, find its parent */
        *target = dir_node->dentry->parent->vnode;
        return 0;
    }
    else{
        /* need to find the child dentry */
        struct list_head *pos;
        list_for_each(pos, &dir_node->dentry->childs){
            Dentry *tmp = (Dentry *)pos;
            if(strcmp(tmp->name, component_name) == 0){
                // TODO: need to check the dir is other filesystem

                /* if the target is a file,  return it */
                *target = tmp->vnode;
                return 0; 
            }
        }
    }
    /* if the target is not found, return error code */
    *target = NULL;
    return -1;
}

int tmpfs_create(struct vnode* dir_node, struct vnode** target, const char* component_name){
    
    return 0;
}

int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name){
    
    return 0;
}
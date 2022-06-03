#include "tmpfs.h"
#include "vfs.h"
#include "allocator.h"
#include "string.h"
#include "mini_uart.h"
struct vnode_operations tmpfs_v_ops;
struct file_operations tmpfs_f_ops;

int tmpfs_register()
{
    // tmpfs_v_ops = (struct vnode_operations)my_malloc(sizeof(struct vnode_operations));
    tmpfs_v_ops.create = tmpfs_create;
    tmpfs_v_ops.lookup = tmpfs_lookup;
    tmpfs_v_ops.ls = tmpfs_ls;
    tmpfs_v_ops.mkdir = tmpfs_mkdir;
    // tmpfs_f_ops = (struct file_operations)my_malloc(sizeof(struct file_operations));
    tmpfs_f_ops.read = tmpfs_read;
    tmpfs_f_ops.write = tmpfs_write;
    // tmpfs_f_ops.open = vfs_open;
    tmpfs_f_ops.close = tmpfs_close;

    // tmpfs_f_ops->lseek64 = tmpfs_lseek64;
    return 0;
}

int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount)
{
    mount->fs = fs;
    mount->root = vnode_create(nullptr,mount,&tmpfs_v_ops,&tmpfs_f_ops,dir_n);
    return 0;
}
char* get_pos_block_addr(int pos,struct tmpfs_inode* inode)
{
    struct tmpfs_block* block;
    int block_idx = pos/FILE_BLOCK_SIZE; // i_th block
    int block_off = pos%FILE_BLOCK_SIZE; //block offset
    // if(pos % FILE_BLOCK_SIZE == 0){
    //     writes_uart_debug("[*]pos: ",FALSE);
    //     busy_wait_writeint(pos,FALSE);
    //     writes_uart_debug(", idx:",FALSE);
    //     busy_wait_writeint(block_idx,FALSE);
    //     writes_uart_debug(", offset:",FALSE);
    //     busy_wait_writeint(block_off,TRUE);
    // }
    
    if(inode->data == nullptr){
        inode->data = my_malloc(sizeof(struct tmpfs_block));
        inode->data->content = my_malloc(FILE_BLOCK_SIZE*sizeof(char));
        inode->data->next = nullptr;
    }
    block = inode->data;
    for (int i = 0; i < block_idx; i++)
    {
        if(block->next==nullptr){ 
            // writes_uart_debug("[*]No next data block",TRUE);
            struct tmpfs_block* new_block = my_malloc(sizeof(struct tmpfs_block));
            new_block->content = my_malloc(FILE_BLOCK_SIZE*sizeof(char));
            new_block->next = nullptr;
            block->next = new_block;
        }
        block = block->next;
    }
    return block->content+block_off;
}
int tmpfs_read(struct file* file, void* buf, size_t len)
{
    struct tmpfs_inode* inode =  (struct tmpfs_inode*)(file->vnode->internal);
    if(inode->type==dir_n){
        writes_uart_debug("[*]read file failed",TRUE);
        ((char*)buf)[0]='\0';
        return errMsg;
    }
    int i,pos;
    writes_uart_debug("[*]Reading ",FALSE);
    writes_uart_debug(inode->name,FALSE);
    writes_uart_debug(" ,len ",FALSE);
    // busy_wait_writeint(len,FALSE);
    writes_uart_debug(" in pos ",FALSE);
    // busy_wait_writeint(file->f_pos,TRUE);
    
    for(pos = file->f_pos,i=0;pos<inode->size && pos<file->f_pos+len;pos++,i++){
        memcpy((char*)(buf+i),get_pos_block_addr(pos,inode),1);
        // strcpy(((char*)buf)+i,get_pos_block_addr(pos,inode));
    }
    // *((char*)(buf+i)) = '\0';
    writes_uart_debug("[*]Read data: ",FALSE);
    writes_uart_debug(buf,TRUE);
    file->f_pos = pos;
    return i;
}
int tmpfs_write(struct file* file, const void* buf, size_t len)
{
    struct tmpfs_inode* inode =  (struct tmpfs_inode*)(file->vnode->internal);
    if(inode->type==dir_n){
        writes_uart_debug("[*]write file failed",TRUE);
        ((char*)buf)[0]='\0';
        return errMsg;
    }
    int i,pos;
    writes_uart_debug("[*]Write data to ",FALSE);
    writes_uart_debug(inode->name,FALSE);
    // writes_uart_debug(buf,FALSE);
    writes_uart_debug(", len:",FALSE);
    // busy_wait_writeint(len,TRUE);
    if(strcmp(inode->name,"uart")==0 && strcmp(inode->parent->name,"dev")==0){
        writes_n_uart((char*)buf,len);
    }
    for(pos = file->f_pos,i=0; pos<file->f_pos+len;pos++,i++){
        // busy_wait_writeint(pos,TRUE);
        memcpy(get_pos_block_addr(pos,inode),((const char*)buf)+i,1);
        // strcpy(get_pos_block_addr(pos,inode),((const char*)buf)+i);
    }
    // *((char*)(get_pos_block_addr(pos,inode))) = '\0';
    inode->size+=len;
    file->f_pos = pos;
    return i;
}
int tmpfs_lookup(struct vnode* dir_node, struct vnode** target,
                const char* component_name)
{
    struct tmpfs_inode* inode = (struct tmpfs_inode*)dir_node->internal;
    if(inode->type == mount_fs)
        inode = dir_node->mount->root->internal;
    inode = inode->child;
    while(inode!=nullptr)
    {
        if(strcmp(component_name,inode->name)==0)
        {
            writes_uart_debug("[*]File look up found ",FALSE);
            writes_uart_debug(inode->name,TRUE);
            *target = inode->vnode;
            return sucessMsg;
        }
        else
        {
            inode = inode->next_sibling;
        }
    }
    writes_uart_debug("[*]File look up not found",TRUE);
    return compNotFound;
}
struct tmpfs_inode* inode_create()
{

    return nullptr;
}
int tmpfs_create(struct vnode* dir_node, struct vnode** target,
                const char* component_name)
{
    struct tmpfs_inode* inode = (struct tmpfs_inode*)(dir_node->internal);
    if(inode->type == dir_n || inode->type == mount_fs){
        if(inode->child==nullptr){
            struct vnode* v_node = vnode_create(dir_node,dir_node->mount,dir_node->v_ops,dir_node->f_ops,file_n);
            strcpy(((struct tmpfs_inode*)(v_node->internal))->name,component_name);
            inode->child = v_node->internal;
            *target = v_node;
        }
        else{
            struct tmpfs_inode* itr = inode->child;
            while(itr->next_sibling!=null){
                itr = itr->next_sibling;
            }
            struct vnode* v_node = vnode_create(dir_node,dir_node->mount,dir_node->v_ops,dir_node->f_ops,file_n);
            strcpy(((struct tmpfs_inode*)(v_node->internal))->name,component_name);
            // ((struct tmpfs_inode*)(v_node->internal))->parent = inode;
            itr->next_sibling = (struct tmpfs_inode*)(v_node->internal);
            *target = v_node;
        }
    }
    else{
        writes_uart_debug("[*]file create failed",TRUE);
    }
    return 0;
}

int tmpfs_close(struct file* file)
{

    free(file);
    return 0;
}

int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target,
              const char* component_name)
{
    struct tmpfs_inode* inode = (struct tmpfs_inode*)(dir_node->internal);
    if(inode->type == dir_n || inode->type == mount_fs){
        // struct tmpfs_inode* itr = inode->child;
        // while(itr->next_sibling!=null){
        //     itr = itr->next_sibling;
        // }
        // struct vnode* v_node = vnode_create(dir_node,dir_node->mount,dir_node->v_ops,dir_node->f_ops,dir_n);
        // strcpy(((struct tmpfs_inode*)(v_node->internal))->name,component_name);
        // itr->next_sibling = (struct tmpfs_inode*)(v_node->internal);
        // // itr->parent = inode;
        // *target = v_node;

        if(inode->child==nullptr){
            struct vnode* v_node = vnode_create(dir_node,dir_node->mount,dir_node->v_ops,dir_node->f_ops,dir_n);
            strcpy(((struct tmpfs_inode*)(v_node->internal))->name,component_name);
            inode->child = v_node->internal;
            *target = v_node;
        }
        else{
            struct tmpfs_inode* itr = inode->child;
            while(itr->next_sibling!=null){
                itr = itr->next_sibling;
            }
            struct vnode* v_node = vnode_create(dir_node,dir_node->mount,dir_node->v_ops,dir_node->f_ops,dir_n);
            strcpy(((struct tmpfs_inode*)(v_node->internal))->name,component_name);
            // ((struct tmpfs_inode*)(v_node->internal))->parent = inode;
            itr->next_sibling = (struct tmpfs_inode*)(v_node->internal);
            *target = v_node;
        }
    }
    else{
        writes_uart_debug("[*]mkdir failed",TRUE);
    }
    return 0;
}

void tmpfs_ls(struct vnode* dir_node)
{
    struct tmpfs_inode* itr;
    if(((struct tmpfs_inode*)(dir_node->internal))->type == dir_n)
        itr = (struct tmpfs_inode*)(dir_node->internal);
    else
        itr = (struct tmpfs_inode*)(dir_node->mount->root->internal);
    writes_uart_debug("[*]Listing files in directory: ",FALSE);
    writes_uart_debug(itr->name,TRUE);
    itr = itr->child;
    while(itr!=nullptr){
        busy_wait_writes(itr->name,FALSE);
        if(itr->type==dir_n)   busy_wait_writes("[DIR]",FALSE);
        else if(itr->type==file_n)   busy_wait_writes("[FILE]",FALSE);
        else if(itr->type==mount_fs)   busy_wait_writes("[MOUNT]",FALSE);
        busy_wait_writes(" ",FALSE);
        itr=itr->next_sibling;
    }
    busy_wait_writes("\r\n",FALSE);
}
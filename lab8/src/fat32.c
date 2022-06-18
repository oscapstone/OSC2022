#include "vfs.h"
#include "fat32.h"
#include "allocator.h"
#include "list.h"
#include "string.h"
#include "mini_uart.h"
#include "sd.h"

struct file_operations *fat32_fops;
struct vnode_operations *fat32_vops;

struct fat32_metadata sd_metadata;

static unsigned int next_cluster(uint32_t cluster_idx);
static uint32_t get_cluster_blk_idx(uint32_t cluster_idx);
static unsigned int get_fat_idx(uint32_t cluster_idx);
static unsigned int get_next_cluster(uint32_t cluster_idx);

int fat32_create(struct vnode* dir_node, struct vnode** target, const char* component_name){
    struct vnode* tmp;
    fat32_lookup(dir_node,&tmp,component_name);
    if(tmp != NULL){
        *target = NULL;
        return -1;
    }
    tmp = allo_vnode();
    strcpy(component_name,tmp->dt->name,16);
    tmp->f_ops = fat32_fops;
    tmp->internal = malloc(0x100);
    delete_last_mem();
    memset(tmp->internal,0,0x100);
    tmp->v_ops = fat32_vops;
    tmp->f_ops = fat32_fops;
    tmp->size = 0;

    tmp->dt->parent = dir_node->dt;
    tmp->dt->childs = NULL;
    tmp->dt->vnode = tmp;
    tmp->dt->type = file;
    *target = tmp;

    struct link_list *parent_ll = dir_node->dt->childs;
    while(parent_ll->next!=NULL)parent_ll = parent_ll->next;
    parent_ll->next = malloc(sizeof(struct link_list));
    delete_last_mem();
    parent_ll->next->entry = tmp->dt;
    parent_ll->next->next = NULL;
    return 0;
}

int fat32_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name){
    struct vnode* temp;
    fat32_lookup(dir_node,&temp,component_name);
    if(temp != NULL){
        *target = NULL;
        return -1;
    }
    struct vnode *tmp = allo_vnode();
    struct dentry *tmp_child = allo_dentry(),*tmp_parent = allo_dentry();
    
    strcpy(component_name,tmp->dt->name,16);
    tmp->f_ops = fat32_fops;
    tmp->v_ops = fat32_vops;
    tmp->size = 0;

    tmp->dt->parent = dir_node->dt;
    tmp->dt->childs = malloc(sizeof(struct link_list));
    delete_last_mem();
    tmp->dt->childs->entry = tmp_child;
    tmp->dt->childs->next = malloc(sizeof(struct link_list));
    delete_last_mem();
    tmp->dt->childs->next->entry = tmp_parent;
    tmp->dt->childs->next->next = NULL; 
    tmp->dt->vnode = tmp;
    tmp->dt->type = directory;
    *target = tmp;

    tmp_child->name[0] = '.';
    tmp_child->name[1] = 0;
    tmp_child->vnode = tmp;
    tmp_child->childs = tmp->dt->childs;
    tmp_child->type = directory;
    tmp_child->parent = tmp->dt->parent;

    tmp_parent->name[0] = '.';
    tmp_parent->name[1] = '.';
    tmp_parent->name[2] = 0;
    tmp_parent->vnode = tmp->dt->parent->vnode;
    tmp_parent->childs = tmp->dt->parent->childs;
    tmp_parent->type = directory;
    tmp_parent->parent = tmp->dt->parent->parent;

    struct link_list *tmp_parent_child = tmp_parent->childs;
    while(tmp_parent_child->next != NULL) tmp_parent_child = tmp_parent_child->next;
    tmp_parent_child->next = malloc(sizeof(struct link_list));
    delete_last_mem();
    tmp_parent_child->next->next = NULL;
    tmp_parent_child->next->entry = tmp->dt;

    return 0;
}

int fat32_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name){
    struct link_list* tmp = dir_node->dt->childs;
    while(tmp != NULL){
        struct dentry* den_tmp = tmp->entry;
        if(strcmp(den_tmp->name,component_name)){
            *target = den_tmp->vnode;
            return 0;
        }
        tmp = tmp->next;
    }
    *target = NULL;
    return -1;
}

int fat32_get_content(unsigned int cluster_idx, struct link_list **pages){
    uint32_t cluster_idx_tmp = cluster_idx;
    struct link_list *now = NULL;
    for(int i=0;cluster_idx_tmp<EOC;i++){
        char *tmp_buf = malloc(512);
        delete_last_mem();
        readblock(get_cluster_blk_idx(cluster_idx_tmp),tmp_buf);
        if(i==0){
            now = malloc(sizeof(struct link_list));
            delete_last_mem();
            now->next = NULL;
            struct page_cache *pc = malloc(sizeof(struct page_cache));
            delete_last_mem();
            pc->modified = 0;
            pc->buf = tmp_buf;
            pc->num = i;
            now->entry = pc;
        }else{
            now->next = malloc(sizeof(struct link_list));
            delete_last_mem();
            now->next->next = NULL;
            struct page_cache *pc = malloc(sizeof(struct page_cache));
            delete_last_mem();
            pc->modified = 0;
            pc->buf = tmp_buf;
            pc->num = i;
            now->next->entry = pc;
            now = now->next;
        }
        cluster_idx_tmp = get_next_cluster(cluster_idx_tmp);
    }
    *pages = now;
    if(now == NULL){
        return -1;
    }else{
        return 0;
    }
}

int fat32_write(struct file* file, const void* buf, size_t len){
    char *internal = file->vnode->internal, *tmp = buf;
    for(int i=0;i<len;i++){
        internal[file->f_pos++] = tmp[i];
    }
    return len;
}

int fat32_read(struct file* file, void* buf, size_t len){
    struct file_internal *internal = file->vnode->internal;
    if(internal->pages != NULL){
        if(file->f_pos >= file->vnode->size) return 0;
        char *tmp = buf;
        size_t read_l = 0;
        int pt = file->f_pos%512;
        int pages_num = file->f_pos/512;
        struct link_list *tmp_ll = internal->pages;
        struct link_list *modified_ll = file->content;
        for(int i=0;i<pages_num;i++){
            if(tmp_ll == NULL) return 0;
            else tmp_ll = tmp_ll->next;
        }
        while(read_l < len){
            if(tmp_ll == NULL) {
                file->f_pos += read_l;
                return read_l;
            }
            for(int i=pt;i<512 && read_l < len;i++){
                char *tmp_buf;
                struct page_cache *pc = tmp_ll->entry;
                if(modified_ll == NULL){                    
                    tmp_buf = pc->buf;
                }else{
                    struct page_cache *modified_page = modified_ll->entry;
                    if(modified_page->num == pc->num){
                        tmp_buf = modified_page->buf;
                    }else{
                        while(modified_page->num<pc->num && modified_ll->next!=NULL){
                            modified_ll = modified_ll->next;
                            modified_page = modified_ll->entry;
                        }
                        tmp_buf = pc->buf;
                    }
                }
                tmp[read_l++] = tmp_buf[i];
                file->f_pos++;
                if(file->f_pos >= file->vnode->size) return read_l;
            } 
            pt = 0;
        }
        return len;
    }else{
        struct file_internal *tmp;
        fat32_get_content(internal->cluster_idx,&tmp);
        internal->pages = tmp;
        struct page_cache *pc = internal->pages;
        return fat32_read(file,buf,len);
    }
}
int fat32_open(struct vnode* file_node, struct file** target){
    struct file* tmp = malloc(sizeof(struct file));
    delete_last_mem();
    tmp->f_pos = 0;
    tmp->f_ops = fat32_fops;
    tmp->vnode = file_node;
    tmp->content = NULL;
    *target = tmp;
    return 0;
}

int fat32_close(struct file* file){
    if(file->f_pos > file->vnode->size) file->vnode->size = file->f_pos; 
    free(file);
    return 0;
}

long fat32_lseek64(struct file* file, long offset, int whence){
    if(file->f_pos > file->vnode->size) file->vnode->size = file->f_pos; 
    if(whence == SEEK_END){
        file->f_pos = file->vnode->size + offset;
        return file->f_pos;
    }else if(whence == SEEK_CUR){
        file->f_pos += offset;
        return file->f_pos;
    }else if(whence == SEEK_SET){
        file->f_pos = offset;
        return file->f_pos;
    }
}

static uint32_t get_cluster_blk_idx(uint32_t cluster_idx) {
    return sd_metadata.data_region_blk_idx +
           (cluster_idx - sd_metadata.first_cluster) * sd_metadata.sector_per_cluster;
}

static unsigned int get_fat_idx(uint32_t cluster_idx){
    return sd_metadata.fat_region_blk_idx + (cluster_idx / FAT_ENTRY_PER_BLOCK);
}

static unsigned int get_next_cluster(uint32_t cluster_idx){
    if(cluster_idx >= EOC) return cluster_idx;
    unsigned int buf[128];
    readblock(get_fat_idx(cluster_idx),buf);
    return buf[cluster_idx%FAT_ENTRY_PER_BLOCK];
}

void traverse_fat32(struct vnode* start){
    uint32_t cluster_idx = start->internal;
    struct fat32_dirent *fd;
    char buf[512];
    do{
        uint32_t d = get_cluster_blk_idx(cluster_idx);
        readblock(d,buf);
        fd = buf;
        for(int i=0;fd[i].name[0]!='\0' && i<16;i++){
            if(fd[i].name[0]==0xE5 || fd[i].name[0]=='.') continue;   //unused
            struct vnode* record;
            char name[32];
            memset(name,0,32);
            int j;
            for(j=0;j<8;j++){
                if(fd[i].name[j] == ' ') break;
                else {
                    name[j] = fd[i].name[j];
                }
            }
            for(int k=0;k<3 && fd[i].ext[k] != ' ';k++){
                if(k == 0) name[j] = '.';
                name[k+j+1] = fd[i].ext[k];
            }
            if(fd[i].attr&(1<<4)) fat32_mkdir(start,&record,name);
            else fat32_create(start,&record,name);
            struct file_internal* internal = malloc(sizeof(struct file_internal));
            delete_last_mem();
            internal->cluster_idx = (uint32_t)fd[i].cluster_high << 16|fd[i].cluster_low;
            internal->pages = NULL;
            record->internal = internal;
            record->size = fd[i].size;
            if(record->dt->type == directory) traverse_fat32(record);
        }

        cluster_idx = get_next_cluster(cluster_idx);
    }while(cluster_idx < EOC);
}

int fat32_mount(struct filesystem* fs, struct mount* mount){
    mount->fs = fs;
    struct vnode *tmp = allo_vnode();
    tmp->f_ops = fat32_fops;
    tmp->v_ops = fat32_vops;
    tmp->mount = NULL;

    struct dentry *tmp_child = allo_dentry();
    struct dentry *tmp_parent = mount->root->dt->childs->next->entry;
    tmp->dt->parent = mount->root->dt->parent;

    tmp->dt->name[0] = 0;
    tmp->dt->childs = malloc(sizeof(struct link_list));
    delete_last_mem();
    tmp->dt->childs->entry = tmp_child;
    tmp->dt->childs->next = malloc(sizeof(struct link_list));
    delete_last_mem();
    tmp->dt->vnode = tmp;
    tmp->dt->type = directory;

    tmp_child->name[0] = '.';
    tmp_child->name[1] = 0;
    tmp_child->vnode = tmp;
    tmp_child->childs = tmp->dt->childs;
    tmp_child->type = directory;
    tmp_child->parent = tmp->dt->parent;
    
    tmp->dt->childs->next->entry = tmp_parent;
    tmp->dt->childs->next->next = NULL;

    mount->root = tmp;
    tmp->internal = sd_metadata.first_cluster;
    traverse_fat32(tmp);
    return 0;
}

void fat32_init(struct fat32_metadata* metadata){
    fat32_vops = malloc(sizeof(struct vnode_operations));
    fat32_fops = malloc(sizeof(struct file_operations));
    delete_last_mem();
    delete_last_mem();
    memcpy(&sd_metadata,metadata,sizeof(struct fat32_metadata));
    fat32_vops->create = fat32_create;
    fat32_vops->lookup = fat32_lookup;
    fat32_vops->mkdir = fat32_mkdir;
    fat32_vops->mknod = NULL;
    fat32_fops->close = fat32_close;
    fat32_fops->lseek64 = fat32_lseek64;
    fat32_fops->open = fat32_open;
    fat32_fops->read = fat32_read;
    fat32_fops->write = fat32_write;
    struct filesystem *fs = malloc(sizeof(struct filesystem));
    delete_last_mem();
    fs->name = malloc(16);
    delete_last_mem();
    memset(fs->name,0,16);
    strcpy("fat32\0",fs->name,6);
    fs->setup_mount = fat32_mount;
    register_filesystem(fs);
}
#include "fs/initrdfs.h"
static struct fentry * root;
/*
struct filesystem_type initrdfs = {
    .fs_name = "initrdfs",
    .mount = initrdfs_mount
};

struct inode_operations initrdfs_i_ops = {
	.create = initrdfs_create,
    .lookup = initrdfs_lookup,
    .mkdir = initrdfs_mkdir
};

struct file_operations initrdfs_f_ops = {
    .lseek64 = initrdfs_lseek64,
    .read =  initrdfs_read,
    .write = initrdfs_write,
    .open = initrdfs_open,
    .flush = initrdfs_flush,
    .release = initrdfs_release
};*/


void initrdfs_init(void* addr){
    INFO("Initrd filesystem start address: %p", addr);
    LOG("Enter initrdfs_int");
    LOG("initrd start at %p\n", addr);
    LOG("initrd first 4 bytes: %x\n", *(uint32_t*)addr);
    root = (struct fentry*)kmalloc(sizeof(struct fentry));
    struct cpio_iter iter;
    char *fname = (char*)kmalloc(2);
    strcpy(fname, "/");

    memset(root, 0, sizeof(struct fentry));
    root->filename = fname;
    root->namesize = 1;
    root->mode = root->mode & FILE_TYPE_DIR; 

    INIT_LIST_HEAD(&root->list);

    // Start parsing initrd in New ASCII Format at addr
    LOG("Start parsing new ascii format's cpio");
    cpio_iter_init(&iter, addr);
    do{
        struct fentry *f = (struct fentry*)kmalloc(sizeof(struct fentry));
        cpio_iter_parse(&iter, f);
        LOG("FILE: %s", f->filename);
        if(cpio_is_tailer(f)) break;
        list_add_tail(&f->list, &root->list);
    }while(1);
    LOG("Leave initrdfs_int");
}

void* fdt_initrdfs_callback(uint32_t token, fdt_node* node, fdt_property* prop, int32_t layer){
    if(prop != NULL){
        if(strcmp(prop->name, "linux,initrd-start") == 0){
            uint64_t initrd_start = bswap32(*(uint32_t*)prop->value) + UPPER_ADDR_SPACE_BASE;
            LOG("linux,initrd-start: %x", (uint32_t)initrd_start);
            initrdfs_init((void*)initrd_start);
        }   
    }
} 

void initrdfs_ls(){
    struct list_head* head = &root->list;
    struct list_head* node;

    list_for_each(node, head){
        struct fentry* f = list_entry(node, struct fentry, list);
        printf("%s\r\n", f->filename); 
    }
}
void initrdfs_cat(){
    struct list_head* head = &root->list;
    struct list_head* node;

    list_for_each(node, head){
        struct fentry* f = list_entry(node, struct fentry, list);
        if((f->mode & FILE_TYPE_MASK) == FILE_TYPE_REGULAR){ 
            printf("Filename : %s\r\n", f->filename); 
            printf("File size: %u\r\n", f->filesize); 
            write_bytes(f->data, f->filesize);
            printf("\r\n");
        }
    }
}

size_t initrdfs_loadfile(char* name, uint8_t* addr, uint64_t offset, uint64_t size){
    struct list_head* head = &root->list;
    struct list_head* node;

	LOG("initrdfs_loadfile(%s, %p, %lu, %lu)", name, addr, offset, size);
    list_for_each(node, head){
        struct fentry* f = list_entry(node, struct fentry, list);
        if((f->mode & FILE_TYPE_MASK) == FILE_TYPE_REGULAR){ 
            if(strcmp(name, f->filename) == 0){
				if(offset <= f->filesize){
					if(offset + size > f->filesize){
						size = f->filesize - offset;
					}
					LOG("Load %lu bytes %s at offset %lu to %p",size , name, offset, addr);
					memcpy(addr, &f->data[offset], size);
					return size; 
				}else{
					LOG("error occur");
				}
                break;
            }
        }
    }
	return 0;
}

size_t initrdfs_filesize(char* name){
    struct list_head* head = &root->list;
    struct list_head* node;

    list_for_each(node, head){
        struct fentry* f = list_entry(node, struct fentry, list);
        if((f->mode & FILE_TYPE_MASK) == FILE_TYPE_REGULAR){ 
            if(strcmp(name, f->filename) == 0){
                return f->filesize;
            }
        }
    }
    return 0;
}


// VFS
/*
struct initrdfs_dir* create_initrdfs_dir(){
    struct initrdfs_dir* ret = kmalloc(sizeof(struct initrdfs_dir));
    memset(ret, 0, sizeof(struct initrdfs_dir));
    return ret;
}

struct dentry* create_initrdfs_file(const char* name, struct dentry* link, umode_t mode){
    char *new_file_name = kmalloc(strlen(name));
    struct dentry* ret;
    struct initrdfs_file * tmp_file;
    strcpy(new_file_name, name);

    // create dentry
    ret = create_dentry(new_file_name, 0, NULL);

    // create and initialzie inode
    ret->d_inode = create_inode(&initrdfs_f_ops, &initrdfs_i_ops, mode);
    if(S_ISREG(mode)){
        ret->d_inode->private_data = kmalloc(sizeof(struct initrdfs_file));
        // create file data space
        tmp_file = ret->d_inode->private_data;
        tmp_file->size = 0;
        tmp_file->data = calloc_page();
    }else if(S_ISDIR(mode)){
        ret->d_inode->private_data = create_initrdfs_dir();
    }else if(S_ISLNK(mode)){
        ret->d_inode->private_data = link; 
    }
    
    return ret;
}

struct dentry* initrdfs_create(struct dentry * parent, const char* new_file_name){
    FS_LOG("initrdfs_create");
    return NULL;
}

struct dentry *initrdfs_lookup(struct dentry *parent, char* target){
    FS_LOG("initrdfs_lookup");
    struct dentry* ent;
    uint64_t daif = local_irq_disable_save();
    struct inode* parent_inode = parent->d_inode;
    struct initrdfs_dir* parent_dir = parent_inode->private_data;

    for(uint64_t i = 0 ; i < MAX_NUM_DIR_ENTRY ; i++){
        ent = parent_dir->entries[i]; 
        if(ent != NULL && strcmp(ent->d_name, target) == 0){
            if(S_ISLNK(ent->d_inode->i_modes)){
                ent = ent->d_inode->private_data;
            }
            FS_LOG("found %s", ent->d_name);
            local_irq_restore(daif);
            return ent;
        }
    } 
    local_irq_restore(daif);
    return NULL;
}

loff_t initrdfs_lseek64(struct file *, loff_t, int){
    FS_LOG("initrdfs_lseek64");
}

long initrdfs_read(struct file *file, char *buf, size_t len, loff_t *offset){
    FS_LOG("initrdfs_read");
    uint64_t daif = local_irq_disable_save();
    struct dentry* d_file = file->f_dentry;
    struct initrdfs_file* tmp_file = d_file->d_inode->private_data;
    char* data = tmp_file->data;
    size_t file_size = tmp_file->size;
    if(*offset + len > file_size) len = file_size - *offset;

    memcpy(buf, &data[*offset], len);
    *offset += len;

    local_irq_restore(daif);
    return len;
}

long initrdfs_write(struct file * file, char * buf, size_t len, loff_t * offset){
    FS_LOG("initrdfs_write");
    return -1;
}

struct file* initrdfs_open(struct dentry* d_file,uint32_t flags,umode_t mode){
    FS_LOG("initrdfs_open");
    return create_file(d_file, flags, mode); 
}
int initrdfs_flush(struct file *){
    FS_LOG("initrdfs_flush");
    return 0;
}
int initrdfs_release(struct inode * inode, struct file * file){
    FS_LOG("initrdfs_release");
    if(file == NULL) return -1;

    file->f_count--;
    if(file->f_count == 0) kfree(file); 
    return 0;
}

void initrdfs_mount_init(struct dentry* root){
    struct list_head* head = &root->list;
    struct list_head* node;

    list_for_each(node, head){
        struct fentry* f = list_entry(node, struct fentry, list);
        if(strcmp(f->fileanme, ".") != 0){

        }
    }
}

struct mount* initrdfs_mount(struct filesystem_type* fs_type, struct dentry* target){
    FS_LOG("initrdfs_mount");
    uint64_t daif = local_irq_disable_save();
    struct dentry* new_root = create_initrdfs_file("", NULL, S_IFDIR), *link; 
    new_root->d_parent = target->d_parent;
    struct inode* root_node = new_root->d_inode;
    struct initrdfs_dir* new_dir;
    struct mount* ret = (struct mount*)kmalloc(sizeof(struct mount));
    target->d_mnt = ret;
    ret->mnt_root = new_root;

    new_dir = new_root->d_inode->private_data;
    new_dir->count += 2;
    link = new_root;
    new_dir->entries[0] = create_initrdfs_file(".", link, S_IFLNK);
    link = target->d_parent;
    new_dir->entries[1] = create_initrdfs_file("..", link, S_IFLNK);
    local_irq_restore(daif);
    return ret;
}


int initrdfs_mkdir(struct dentry * parent, const char * target, umode_t mode){
    return -1;
}*/

#include "utils.h"
#include "types.h"
#include "debug/debug.h"
#include "peripherals/mini_uart.h"
static struct fentry * root;

void initrdfs_init(void* addr){
    INFO("Initrd filesystem start address: %p", addr);
    LOG("Enter initrdfs_int");
    LOG("initrd start at %p\n", addr);
    LOG("initrd first 4 bytes: %x\n", *(uint32_t*)addr);
    root = (struct fentry*)simple_malloc(sizeof(struct fentry));
    struct cpio_iter iter;
    char *fname = (char*)simple_malloc(2);
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
        struct fentry *f = (struct fentry*)simple_malloc(sizeof(struct fentry));
        cpio_iter_parse(&iter, f);
        LOG("FILE: %s", f->filename);
        if(cpio_is_tailer(f)) break;
        list_add(&f->list, &root->list);
    }while(1);
    LOG("Leave initrdfs_int");
}
void* fdt_initrdfs_callback(uint32_t token, fdt_node* node, fdt_property* prop, int32_t layer){
    if(prop != NULL){
        if(strcmp(prop->name, "linux,initrd-start") == 0){
            uint64_t initrd_start = bswap32(*(uint32_t*)prop->value);
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


#include "initramfs.h"
#include "string.h"
#include "memory.h"
#include "cpio.h"
#include "utils.h"

file_operations_t initramfs_file_operations = {initramfs_write, initramfs_read, initramfs_open, initramfs_close, vfs_lseek64, initramfs_getsize};
vnode_operations_t initramfs_vnode_operations = {initramfs_lookup, initramfs_create, initramfs_mkdir};

filesystem_t *get_initramfs() {
    filesystem_t *initramfs = (filesystem_t*)malloc(sizeof(filesystem_t));
    initramfs->name = "initramfs";
    initramfs->setup_mount = setup_initramfs_mount;
    return initramfs;
}

int setup_initramfs_mount(filesystem_t *fs, mount_t *mount) {
    uart_printf("Mount initramfs\n");
    mount->root = create_initramfs_vnode(NULL, nt_dir);
    mount->fs = fs;

    initramfs_internal_t *root_internal = (initramfs_internal_t*)mount->root->internal;
    // add all files under root of initramfs
    
    char filename[MAX_PATHNAME];
    char *p = (char*)INITRAMFS_ADDR;
    char *initram_address = (char*)INITRAMFS_ADDR;
    uint64 namesize, filesize;
    uint64 idx = 0;
        
    do {
        filesize = hex2num(p + 54, 8);
        namesize = hex2num(p + 94, 8);
        p = extract_section(filename, p + 110, initram_address, namesize); // read filename
        
        if(strcmp(filename, "TRAILER!!!")) { // ending string
            break;
        }

        if(strcmp(filename, ".")) { // always the first file without any content
            continue;
        }

        // create entry under root
        vnode_t *new_entry = create_initramfs_vnode(NULL, nt_file);
        initramfs_internal_t *new_entry_internal = (initramfs_internal_t*)new_entry->internal;
        free((void*)new_entry_internal->data);

        uart_printf("(initramfs) Add %s (0x%x)\n", filename, p);
        new_entry_internal->data = p;
        new_entry_internal->datasize = filesize;
        strcpy(new_entry_internal->name, filename);
        root_internal->entry[idx++] = new_entry;

        // jump to next file
        p += filesize;
        if((*p) == 0x00) { //4 bytes padding if necessary 
            p += 4 - ((uint64)(p - initram_address) % 4);
        }

    } while(strcmp(filename, "TRAILER!!!") == 0);
    

    return 0;
}

vnode_t *create_initramfs_vnode(mount_t *mount, enum NodeType ntype) {
    vnode_t *vn = (vnode_t*)malloc(sizeof(vnode_t));
    vn->mount = mount;
    vn->v_ops = &initramfs_vnode_operations;
    vn->f_ops = &initramfs_file_operations;

    vn->internal = malloc(sizeof(initramfs_internal_t));
    memset(vn->internal, 0, sizeof(initramfs_internal_t));
    
    initramfs_internal_t *vn_internal = (initramfs_internal_t*)vn->internal;
    vn_internal->ntype = ntype;
    vn_internal->data = (char*)malloc(MAX_FILE_SIZE);
    vn_internal->datasize = MAX_FILE_SIZE;
    return vn;
}

int initramfs_write(file_t *file, const void *buf, size_t len) {
    return -1;
}

int initramfs_read(file_t *file, void *buf, size_t len) {
    initramfs_internal_t *internal = (initramfs_internal_t*)file->vnode->internal;
    len = (file->f_pos + len < internal->datasize) ? len : internal->datasize - file->f_pos;
    // uart_printf("(initramfs) read from 0x%x(%d) to 0x%x, %d bytes\n", internal->data + file->f_pos, file->f_pos, buf, len);
    memcpy(buf, internal->data + file->f_pos, len);
    file->f_pos += len;

    return len;
}

int initramfs_open(vnode_t *file_node, file_t **target) {
    (*target)->vnode = file_node;
    (*target)->f_ops = file_node->f_ops;
    (*target)->f_pos = 0;
    return 0;
}

int initramfs_close(file_t *file) {
    free((void*)file);
    return 0;
}

int initramfs_getsize(vnode_t *target) {
    return ((initramfs_internal_t*)target->internal)->datasize;
}

int initramfs_lookup(vnode_t *dir, vnode_t **target, const char *component_name) {
    for(int i = 0; i < MAX_ENTRY; i++) {
        vnode_t *ent = ((initramfs_internal_t*)dir->internal)->entry[i];
        if(ent && strcmp(component_name, ((initramfs_internal_t*)ent->internal)->name)) {
            *target = ent;
            return 0;
        }
    }

    uart_printf("[ERROR](initramfs) fail to lookup\n");
    return -1;
}

int initramfs_create(vnode_t *dir, vnode_t **target, const char *component_name) {
    return -1;
}

int initramfs_mkdir(vnode_t *dir, vnode_t **target, const char *component_name) {
    return -1;
}


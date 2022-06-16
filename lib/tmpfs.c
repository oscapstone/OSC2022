#include "tmpfs.h"
#include "string.h"
#include "memory.h"

file_operations_t tmpfs_file_operations = {tmpfs_write, tmpfs_read, tmpfs_open, tmpfs_close, vfs_lseek64, tmpfs_getsize};
vnode_operations_t tmpfs_vnode_operations = {tmpfs_lookup, tmpfs_create, tmpfs_mkdir};

filesystem_t *get_tmpfs() {
    filesystem_t *tmpfs = (filesystem_t*)malloc(sizeof(filesystem_t));
    tmpfs->name = "tmpfs";
    tmpfs->setup_mount = setup_tmpfs_mount;
    return tmpfs;
}

int setup_tmpfs_mount(filesystem_t *fs, mount_t *mount) {
    uart_printf("Mount tmpfs\n");
    mount->root = create_tmpfs_vnode(NULL, nt_dir);
    mount->fs = fs;
    return 0;
}

vnode_t *create_tmpfs_vnode(mount_t *mount, enum NodeType ntype) {
    vnode_t *vn = (vnode_t*)malloc(sizeof(vnode_t));
    vn->mount = mount;
    vn->v_ops = &tmpfs_vnode_operations;
    vn->f_ops = &tmpfs_file_operations;

    vn->internal = malloc(sizeof(tmpfs_internal_t));
    memset(vn->internal, 0, sizeof(tmpfs_internal_t));
    
    tmpfs_internal_t *vn_internal = (tmpfs_internal_t*)vn->internal;
    vn_internal->ntype = ntype;
    vn_internal->data = (char*)malloc(MAX_FILE_SIZE);
    vn_internal->datasize = 0;
    return vn;
}

int tmpfs_write(file_t *file, const void *buf, size_t len) {
    tmpfs_internal_t *internal = (tmpfs_internal_t*)file->vnode->internal;
    len = (file->f_pos + len < MAX_FILE_SIZE) ? len : MAX_FILE_SIZE - file->f_pos;
    memcpy(internal->data + file->f_pos, buf, len);
    file->f_pos += len;
    internal->datasize = file->f_pos;

    return len;
}

int tmpfs_read(file_t *file, void *buf, size_t len) {
    tmpfs_internal_t *internal = (tmpfs_internal_t*)file->vnode->internal;
    len = (file->f_pos + len < internal->datasize) ? len : internal->datasize - file->f_pos;
    memcpy(buf, internal->data + file->f_pos, len);
    file->f_pos += len;

    return len;
}

int tmpfs_open(vnode_t *file_node, file_t **target) {
    (*target)->vnode = file_node;
    (*target)->f_ops = file_node->f_ops;
    (*target)->f_pos = 0;
    return 0;
}

int tmpfs_close(file_t *file) {
    free((void*)file);
    return 0;
}

int tmpfs_getsize(vnode_t *target) {
    return ((tmpfs_internal_t*)target->internal)->datasize;
}

int tmpfs_lookup(vnode_t *dir, vnode_t **target, const char *component_name) {
    for(int i = 0; i < MAX_ENTRY; i++) {
        vnode_t *ent = ((tmpfs_internal_t*)dir->internal)->entry[i];
        if(ent && strcmp(component_name, ((tmpfs_internal_t*)ent->internal)->name)) {
            // uart_printf("[MATCH] (tmpfs) %s, %s\n", component_name, ((tmpfs_internal_t*)ent->internal)->name);
            *target = ent;
            return 0;
        }
    }

    // uart_printf("[ERROR] (tmpfs) fail to lookup: %s\n", component_name);
    return -1;
}

int tmpfs_create(vnode_t *dir, vnode_t **target, const char *component_name) {
    tmpfs_internal_t *internal = (tmpfs_internal_t*)dir->internal;

    if(internal->ntype != nt_dir) {
        uart_printf("[ERROR] (tmpfs) fail to create, dir is not directory\n");
        return -1;
    }

    int sel = -1;
    for(int i = 0; i < MAX_ENTRY; i++) {
        vnode_t *ent = internal->entry[i];
        if(!ent) {
            if(sel == -1) sel = i;
        }
        else {
            tmpfs_internal_t *ent_internal = (tmpfs_internal_t*)ent->internal;
            if(strcmp(component_name, ent_internal->name) && ent_internal->ntype == nt_file) {
                uart_printf("[ERROR] (tmpfs) fail to create, target exists\n");
                return -1;
            }
        }
    }

    if(sel == -1) {
        uart_printf("[ERROR] (tmpfs) fail to create, dir is full\n");
        return -1;
    }
    
    internal->entry[sel] = create_tmpfs_vnode(NULL, nt_file);
    strcpy(((tmpfs_internal_t*)internal->entry[sel]->internal)->name, component_name);
    (*target) = internal->entry[sel];

    return 0;
}

int tmpfs_mkdir(vnode_t *dir, vnode_t **target, const char *component_name) {
    tmpfs_internal_t *internal = dir->internal;

    if(internal->ntype != nt_dir) {
        uart_printf("[ERROR] (tmpfs) fail to mkdir, dir is not directory\n");
        return -1;
    }

    int sel = -1;
    for(int i = 0; i < MAX_ENTRY; i++) {
        vnode_t *ent = internal->entry[i];
        if(!ent) {
            if(sel == -1) sel = i;
        }
        else {
            tmpfs_internal_t *ent_internal = (tmpfs_internal_t*)ent->internal;
            if(strcmp(component_name, ent_internal->name) && ent_internal->ntype == nt_dir) {
                uart_printf("[ERROR] (tmpfs) fail to mkdir, target exists\n");
                return -1;
            }
        }
    }

    if(sel == -1) {
        uart_printf("[ERROR] (tmpfs) fail to mkdir, dir is full\n");
        return -1;
    }

    internal->entry[sel] = create_tmpfs_vnode(NULL, nt_dir);
    strcpy(((tmpfs_internal_t*)internal->entry[sel]->internal)->name, component_name);
    (*target) = internal->entry[sel];

    return 0;
}


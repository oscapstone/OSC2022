#include "tmpfs.h"

#include "mem.h"
#include "cpio.h"
#include "uart.h"
#include "utils.h"

struct mount root_fs;
struct mount* rootfs = &root_fs;

vnode **dev;

void check_tree(vnode* root) {
    File_Info *info = (File_Info*)root->internal;
    vnode** child = info->data;
    for (int i=0; i<TMPFS_DIR_LEN; i++) {
        if (child[i] != NULLPTR) {
            vnode *target = child[i];
            File_Info *target_info = (File_Info*)target->internal;
            if (target_info->mode == MODE_DIR) {
                printf("%s-->%s\n", info->name, target_info->name);
                check_tree(target);
            }
            else if (target_info->mode == MODE_FILE)
                printf("%s-->%s\n", info->name, target_info->name);
        }
        else
            break;
    }
}

int tmpfs_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name) {
	File_Info* parent_info = (File_Info*)dir_node->internal;
	if (parent_info->mode != MODE_DIR) {
		printf("Not directory\n");
		while (1) {}
	}

    if (!strcmp(component_name, ".")) {
        *target = dir_node;
        return 0;
    }
    else if (!strcmp(component_name, "..")) {
        *target = dir_node->parent;
        return 0;
    }

    vnode **childs = (vnode**)parent_info->data;
    for (int i=0; i<TMPFS_DIR_LEN; i++) {
        vnode *child = childs[i];
        if (child == NULLPTR)
            continue;
        File_Info* child_info = (File_Info*)child->internal;
        if (!strcmp(child_info->name, component_name)) {
            if (target != NULLPTR)
                *target = child;
            return 0;
        }
    }
    printf("Fail: file not exist\n");
	return 1;
}

int tmpfs_create(struct vnode* dir_node, struct vnode** target, const char* component_name) {
	vnode* child = vnode_create(dir_node, (char*)component_name);
    File_Info *child_info = (File_Info*)child->internal;
    strcpy(child_info->name, (char*)component_name);
    child_info->mode = MODE_FILE;
    child_info->size = 0;
    child_info->data = cmalloc(MAX_FILE_SIZE);
	if (target != NULLPTR)
		*target = child;
	return 0;
}

int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name) {
	vnode* child = vnode_create(dir_node, (char*)component_name);
    File_Info *child_info = (File_Info*)child->internal;
    strcpy(child_info->name, (char*)component_name);
    child_info->mode = MODE_DIR;
    child_info->size = 0;
    child_info->data = (vnode**)kmalloc(TMPFS_DIR_LEN * 8);
    for (int i=0; i<TMPFS_DIR_LEN; i++)
        ((vnode**)child_info->data)[i] = NULLPTR;

    if (target != NULLPTR)
		*target = child;
	return 0;
}

int tmpfs_read(struct file* file, void* buf, size_t len) {
	vnode* node = file->vnode;
	File_Info *info = (File_Info*)node->internal;
	if (info->mode != MODE_FILE) {
        printf("Not file\n");
		while (1) {}
	}

    char* buffer = (char*)buf;
    char* data = (char*)info->data;
    int max_len = (info->size - file->f_pos) > len ? len : (info->size - file->f_pos);

    int log = file->f_pos;
    for (int i=0; i<max_len; i++) {
        buffer[i] = data[log + i];
        file->f_pos++;
    }
    
    return max_len;
}

int tmpfs_write(struct file* file, const void* buf, size_t len) {
	vnode* node = file->vnode;
	File_Info *info = (File_Info*)node->internal;
	if (info->mode != MODE_FILE) {
        printf("Not file\n");
		while (1) {}
	}

    char* buffer = (char*)buf;
    char* data = (char*)info->data;
    int max_len = (MAX_FILE_SIZE - file->f_pos) > len ? len : (MAX_FILE_SIZE - file->f_pos);

    int log = file->f_pos;
    for (int i=0; i<max_len; i++) {
        data[log + i] = buffer[i];
        file->f_pos++;
    }
    info->size = file->f_pos;

    return max_len;
}

int tmpfs_open(struct vnode* dir_node, const char* pathname, int flags, struct file** target) {
    vnode **target_file = &dir_node;
    char *tmp_path = (char*)pathname;
    char name[TMPFS_NAME_LEN];
    while (*tmp_path) {
        tmp_path = parse_path(tmp_path, name);
        if (tmpfs_lookup(*target_file, target_file, name)) {
            if (flags == O_CREAT) {
                printf("create file: %s\n", name);
                tmpfs_create(*target_file, target_file, name);
                break;
            }
            else
                return 1;
        }
    }
    struct file* file = (struct file*)kmalloc(sizeof(struct file));
    file->vnode = *target_file;
    file->f_ops = (*target_file)->f_ops;
    file->f_pos = 0;
    file->flags = flags;

    *target = file;
    return 0;
}

int tmpfs_close(struct file* file) {
	kfree(file);
	return 0;
}

int tmpfs_setup_node(vnode* root) {
	root->v_ops = (struct vnode_operations*)kmalloc(sizeof(struct vnode_operations));
	root->v_ops->lookup = tmpfs_lookup;
	root->v_ops->create = tmpfs_create;
	root->v_ops->mkdir = tmpfs_mkdir;
    root->f_ops=(struct file_operations*)kmalloc(sizeof(struct file_operations));
	root->f_ops->write = tmpfs_write;
	root->f_ops->read = tmpfs_read;
	root->f_ops->open = tmpfs_open;
	root->f_ops->close = tmpfs_close;
    root->parent = root;

    root->internal = (File_Info*)kmalloc(sizeof(File_Info));
    File_Info *info = (File_Info*)root->internal;
    strcpy(info->name, "/");
    info->mode = MODE_DIR;
    info->size = 0;
    info->data = (vnode**)kmalloc(TMPFS_DIR_LEN * 8);
    for (int i=0; i<TMPFS_DIR_LEN; i++)
        ((vnode**)info->data)[i] = NULLPTR;

    if (root == rootfs->root)
        build_file_tree(root);

    return 0;
}



int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount) {
    mount->root = (vnode*)kmalloc(sizeof(vnode));
    mount->fs = fs;
    mount->root->mount = mount;
    tmpfs_setup_node(mount->root);

	return 0;
}

vnode* vnode_create(vnode* parent, char* filename) {
	File_Info* parent_info = (File_Info*)parent->internal;
	if (parent_info->mode != MODE_DIR) {
		printf("Not directory\n");
		while (1) {}
	}

	int file_num;
	if (parent_info->size < TMPFS_DIR_LEN)
		file_num = parent_info->size++;
	else {
		printf("No enough space\n");
		while (1) {}
	}
	
    vnode **childs = (vnode**)parent_info->data;
    for (int i=0; i<TMPFS_DIR_LEN; i++) {
        vnode *target = childs[i];
        if (target == NULLPTR)
            continue;
        File_Info* target_info = (File_Info*)target->internal;
        if (!strcmp(target_info->name, filename)) {
            printf("Fail: file exist\n");
            return NULLPTR;
        }
    }

	vnode* child = (vnode*)kmalloc(sizeof(vnode));
	child->mount = parent->mount;
	child->v_ops = parent->v_ops;
	child->f_ops = parent->f_ops;
	child->internal = (File_Info*)kmalloc(sizeof(File_Info));
    child->parent = parent;

	childs[file_num] = child;

	return child;
}

int initramfs_read_only() {
    printf("Error: read-only\n");
    return -1;
}

int initramfs_setup_node(vnode* root) {
	root->v_ops->create = initramfs_read_only;
	root->v_ops->mkdir = initramfs_read_only;
	root->f_ops->write = initramfs_read_only;

    return 0;
}

int initramfs_setup_mount(struct filesystem* fs, struct mount* mount) {
    mount->root = (vnode*)kmalloc(sizeof(vnode));
    mount->fs = fs;
    mount->root->mount = mount;
    tmpfs_setup_node(mount->root);
    initramfs_setup_node(mount->root);

	return 0;
}

int uartfs_read(struct file* file, void* buf, size_t len) {
	vnode* node = file->vnode;
	File_Info *info = (File_Info*)node->internal;
	if (info->mode != MODE_FILE) {
        printf("Not file\n");
		while (1) {}
	}

    char* buffer = (char*)buf;
    for (int i=0; i<len; i++) {
        buffer[i] = async_uart_getc();
    }
    
    return len;
}

int uartfs_write(struct file* file, const void* buf, size_t len) {
	vnode* node = file->vnode;
	File_Info *info = (File_Info*)node->internal;
	if (info->mode != MODE_FILE) {
        printf("Not file\n");
		while (1) {}
	}

    char* buffer = (char*)buf;
    for (int i=0; i<len; i++) {
        uart_send(buffer[i]);
    }

    return len;
}

int uartfs_setup_node(vnode* root) {
    root->f_ops=(struct file_operations*)kmalloc(sizeof(struct file_operations));
	root->f_ops->write = uartfs_write;
	root->f_ops->read = uartfs_read;
    root->parent = root;

    root->internal = (File_Info*)kmalloc(sizeof(File_Info));
    File_Info *info = (File_Info*)root->internal;
    strcpy(info->name, "uart");
    info->mode = MODE_FILE;
    info->size = 0;
    info->data = 0;

    return 0;
}

int uartfs_setup_mount(struct filesystem* fs, struct mount* mount) {
    mount->root = (vnode*)kmalloc(sizeof(vnode));
    mount->fs = fs;
    mount->root->mount = mount;
    tmpfs_setup_node(mount->root);
    uartfs_setup_node(mount->root);

	return 0;
}

void setup_uart_fs() {
    tmpfs_mkdir(rootfs->root, dev, "dev");
    tmpfs_create(*dev, dev, "uart");
    
    struct mount *new_mount = kmalloc(sizeof(struct mount));
    register_filesystem("uartfs");
    struct filesystem *mount_fs = find_fs("uartfs");
    mount_fs->setup_mount(mount_fs, new_mount);
    vnode *mount_vnode = new_mount->root;

    new_mount->root->parent = (*dev)->parent;

    File_Info* parent_info = (File_Info*)((*dev)->parent)->internal;
    vnode **childs = (vnode**)parent_info->data;
    for (int i=0; i<TMPFS_DIR_LEN; i++) {
        if (childs[i] == *dev) {
            childs[i] = mount_vnode;
            break;
        }
    }

    *dev = mount_vnode;
}

void setup_uart_fd(struct file **fd_table) {
    struct file* file = (struct file*)kmalloc(sizeof(struct file));
    file = (struct file*)kmalloc(sizeof(struct file));
    file->vnode = *dev;
    file->f_ops = (*dev)->f_ops;
    file->f_pos = 0;
    file->flags = 0;
    fd_table[0] = file;
    fd_table[1] = file;
    fd_table[2] = file;
}
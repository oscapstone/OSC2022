#include "vfs.h"

#include "utils.h"
#include "mem.h"
#include "tmpfs.h"

struct filesystem *find_fs(const char *filesystem) {
	struct filesystem *fs = NULLPTR;
	for (int i=0; i<FILE_SYSTEM_LEN; i++) {
		if (fspool[i] != 0 && !strcmp((char*)fspool[i]->name, (char*)filesystem)) {
			fs = fspool[i];
			break;
		}
	}
	return fs;
}

struct filesystem *alloc_fs(const char *filesystem) {
	for (int i=0; i<FILE_SYSTEM_LEN; i++) {
		if (fspool[i] == 0) {
			fspool[i] = kmalloc(sizeof(struct filesystem));
			return fspool[i];
		}
	}

	// printf("Not enough file system space\n");
	return NULLPTR;
}

int register_filesystem(const char *filesystem) {
	// setup file system
	struct filesystem *fs = find_fs(filesystem);
	if (fs == NULLPTR) {
		fs = alloc_fs(filesystem);
		strcpy((char*)fs->name, (char*)filesystem);
	}
	
	if (!strcmp((char*)fs->name, "tmpfs"))
		fs->setup_mount = tmpfs_setup_mount;
	else if (!strcmp((char*)fs->name, "initramfs")) 
		fs->setup_mount = initramfs_setup_mount;
	else if (!strcmp((char*)fs->name, "uartfs"))
		fs->setup_mount = uartfs_setup_mount;
	else
		return -1;

	return 0;
}

int vfs_open(struct vnode* dir_node, const char* pathname, int flags, struct file** target) {
	return dir_node->f_ops->open(dir_node, pathname, flags, target);
}

int vfs_close(struct file* file) {
	return file->f_ops->close(file);
}

int vfs_write(struct file* file, const void* buf, size_t len) {
	return file->f_ops->write(file, buf, len);
}

int vfs_read(struct file* file, void* buf, size_t len) {
	return file->f_ops->read(file, buf, len);	
}

int vfs_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name) {
	return dir_node->v_ops->mkdir(dir_node, target, component_name);
}
int vfs_create(struct vnode* dir_node, struct vnode** target, const char* component_name) {
	return dir_node->v_ops->create(dir_node, target, component_name);
}
int vfs_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name) {
	return dir_node->v_ops->lookup(dir_node, target, component_name);
}

char *parse_path(char *path, char *target) {
    if (target == NULLPTR) {
        if (path[0] == '/')
            return path+1;
        else
            return path;
	}

	int absolute;
	absolute = path[0] == '/' ? 1 : 0;
	
    unsigned int size = 0;
    for (int i=1; i<TMPFS_NAME_LEN; i++) {
		if (path[i] == '/' || path[i] == '\0') {
			size = i - absolute;
			break;
		}
    }

    strncpy(target, path+absolute, size);
    return path + size + absolute;
}

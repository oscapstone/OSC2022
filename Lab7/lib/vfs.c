#include "vfs.h"
#include "tmpfs.h"
#include "utils.h"
#include "mini_uart.h"
#include "allocator.h"

char buffer[16];

int register_filesystem(filesystem* fs, const char* fs_name) {
    if (compare_string(fs_name, "tmpfs") == 0) {
        char* name = (char*)kmalloc(6);
        for (int i = 0; i < 6; ++i)
            name[i] = fs_name[i];
        fs->name = name;
        fs->setup_mount = tmpfs_setup;
    }
    else {
        uart_printf("[ERROR][register_filesystem] fs not supported!\n");
        while(1) {}
    }
    return 0;
}

int vfs_open(const char* pathname, int flags, file** target) {
    *target = kmalloc(sizeof(file));
	vnode* dir = rootfs->root;
	vnode* file_node;
    int ret = vfs_lookup(pathname, &file_node);
    if (ret == FAIL)
        return FAIL;
    else if (ret == FILE_NOT_EXIST && !(flags & O_CREAT))
        return FAIL;
    else if (ret == FILE_NOT_EXIST) 
		dir->v_ops->create(dir, &file_node, buffer);
	(*target)->node = file_node;
	(*target)->f_pos = 0;
	(*target)->flags = flags;
    (*target)->f_ops = dir->f_ops;
	return (*target)->f_ops->open(file_node, target);
}

int vfs_close(file* f) {
    if (!f) {
        uart_printf("[ERROR][tmpfs_close] Null pointer!\n");
        return f->f_ops->close(f);
    }
    kfree(f);
	return SUCCESS;
}

int vfs_read(file* f, void* buf, size_t len) {
	return f->f_ops->read(f, buf, len);
}

int vfs_write(file* f, const void* buf, size_t len) {
	return f->f_ops->write(f, buf, len);
}

int vfs_mkdir(const char* pathname);
int vfs_mount(const char* target, const char* filesystem);

int vfs_lookup(const char* pathname, vnode** target) {
    vnode* dir = rootfs->root;
    char prefix[PREFIX_LEN];
    pathname = slashIgnore(pathname, prefix, PREFIX_LEN);
	while (1) {
		pathname = slashIgnore(pathname, prefix, PREFIX_LEN);
		int idx = dir->v_ops->lookup(dir, target, prefix);
		if (pathname) {
			if (idx >= 0)
				dir = *target;
			else {
				uart_printf("[ERROR][vfs_lookup] Something went wrong!\n");
                return FAIL;
            }
		} else {
			if (idx >= 0)
				return SUCCESS;  // already exist
			else {
                for (int i = 0; i < PREFIX_LEN; ++i)
                    buffer[i] = prefix[i];
                return FILE_NOT_EXIST;
            }
		}
	}
	uart_printf("[ERROR][vfs_lookup] Should not reach here!\n");
    return FAIL;
}
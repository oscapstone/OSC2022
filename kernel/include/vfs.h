#ifndef VFS_H_
#define VFS_H_

#include <stddef.h>
#include <list.h>

enum dentry_type {
    D_DIR,
    D_FILE
};
#define O_CREAT 1
#define MAX_PATHNAME_LEN 256
#define MAX_DATA_LEN 512
#define EOF (-1)

// file handle
typedef struct file {
	struct vnode* vnode;
	size_t f_pos;  // RW position of this file handle
	struct file_operations* f_ops;
	int flags;
}File;

typedef struct vnode {
	struct dentry* dentry;
	struct vnode_operations* v_ops;
	struct file_operations* f_ops;
	void* internal;
}VNode;

typedef struct dentry {
    struct list_head list;
    struct list_head childs;
	struct dentry *parent;
    struct mount* mount;
    char* name;
    enum dentry_type type;
    struct vnode* vnode;
}Dentry;

typedef struct mount {
	struct filesystem* fs;
	struct dentry* root_dentry;
}Mount;

typedef struct filesystem {
	char* name;
	int (*setup_mount)(struct filesystem* fs, struct mount* mount);
}FileSystem;

struct file_operations {
	int (*write)(struct file* file, const void* buf, size_t len);
	int (*read)(struct file* file, void* buf, size_t len);
	int (*open)(struct vnode* file_node, struct file** target);
	int (*close)(struct file* file);
	long (*lseek64)(struct file* file, long offset, int whence);
};

struct vnode_operations {
	int (*lookup)(struct vnode* dir_node, struct vnode** target,
					const char* component_name);
	int (*create)(struct vnode* dir_node, struct vnode** target,
					const char* component_name);
	int (*mkdir)(struct vnode* dir_node, struct vnode** target,
				const char* component_name);
};




void rootfs_init();
int register_filesystem(FileSystem* fs);
int vfs_open(const char* pathname, int flags, struct file** target);
int vfs_lookup(const char* pathname, Dentry **target_dentry, VNode **target_vnode, char *component_name);
// int vfs_lookup(const char *pathname, Dentry *target_dentry, VNode *target_vnode);
void find_component_name(const char *pathname, char *target_name, char delimiter);

int vfs_close(struct file* file);
int vfs_write(struct file* file, const void* buf, size_t len);
int vfs_read(struct file* file, void* buf, size_t len);
int vfs_mkdir(const char* pathname);
int vfs_ls(const char* pathname);
int vfs_mount(const char* target, const char* filesystem);

#endif
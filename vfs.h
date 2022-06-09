#ifndef VFS_h
#define VFS_h

#define size_t unsigned long
#define FILE_SYSTEM_LEN 16
#define O_CREAT 00000100

struct filesystem *fspool[FILE_SYSTEM_LEN];

typedef struct vnode {
	struct mount* mount;
	struct vnode_operations* v_ops;
	struct file_operations* f_ops;
	void* internal;
	struct vnode* parent;
} vnode;

// file handle
struct file {
	struct vnode* vnode;
	size_t f_pos;  // RW position of this file handle
	struct file_operations* f_ops;
	int flags;
};

struct mount {
	struct vnode* root;
	struct filesystem* fs;
};

struct filesystem {
	const char* name[FILE_SYSTEM_LEN];
	int (*setup_mount)(struct filesystem* fs, struct mount* mount);
};

struct file_operations {
	int (*write)(struct file* file, const void* buf, size_t len);
	int (*read)(struct file* file, void* buf, size_t len);
	int (*open)(struct vnode* dir_node, const char* pathname, int flags, struct file** target);
	int (*close)(struct file* file);
	// long lseek64(struct file* file, long offset, int whence);
};

struct vnode_operations {
	int (*lookup)(struct vnode* dir_node, struct vnode** target,
				const char* component_name);
	int (*create)(struct vnode* dir_node, struct vnode** target,
				const char* component_name);
	int (*mkdir)(struct vnode* dir_node, struct vnode** target,
				const char* component_name);
};

struct filesystem *find_fs(const char *filesystem);
int register_filesystem(const char *filesystem);
char *parse_path(char *path, char *target);
#endif
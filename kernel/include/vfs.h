#ifndef VFS_H_
#define VFS_H_

#include <stddef.h>
#include <list.h>

enum dentry_type {
    D_DICT,
    D_FILE
};

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
    struct mount* mount;
    char* name;
    enum dentry_type type;
    struct vnode* vnode;
}Dentry;

typedef struct mount {
  struct filesystem* fs;
  struct dentry* root;
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




void vfs_init();
int register_filesystem(FileSystem* fs);
int vfs_open(const char* pathname, int flags, struct file** target);
int vfs_close(struct file* file);
int vfs_write(struct file* file, const void* buf, size_t len);
int vfs_read(struct file* file, void* buf, size_t len);
int vfs_mkdir(const char* pathname);
int vfs_mount(const char* target, const char* filesystem);
int vfs_lookup(const char* pathname, struct vnode** target);

#endif